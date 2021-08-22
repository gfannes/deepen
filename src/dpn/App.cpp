#include <dpn/App.hpp>
#include <dpn/log.hpp>
#include <dpn/input/from_file.hpp>
#include <gubg/naft/Document.hpp>
#include <gubg/mss.hpp>
#include <fstream>
#include <filesystem>
#include <list>

namespace dpn { 

    bool App::load_config()
    {
        MSS_BEGIN(bool);

        std::string filepath;
        MSS(config::get_default_config_filepath(filepath));

        MSS(config_.load_from_file(filepath));

        MSS_END();
    }

    bool App::run()
    {
        MSS_BEGIN(bool);

        if (false) {}
        else if (options_.command)
        {
            const auto &command = *options_.command;

            gubg::naft::Document doc{std::cout};
            unsigned int ok_count = 0;
            std::list<std::string> fails;
            for (const auto &root: config_.roots)
            {
                try
                {
                    std::filesystem::current_path(root);
                    auto run_node = doc.node("Run");
                    run_node.attr("command", command);
                    run_node.attr("pwd", std::filesystem::current_path().string());
                    run_node.text("\n");

                    std::flush(std::cout);
                    const auto rc = std::system(command.c_str());
                    {
                        auto status_node = run_node.node("Status"); 
                        status_node.attr("rc", rc);
                        if (rc == 0)
                            ++ok_count;
                        else
                            fails.push_back(root);
                    }
                }
                catch (...)
                {
                    log::error() << "Path `" << root << "` does not exist" << std::endl;
                    fails.push_back(root);
                }
                doc.text("\n");
            }
            auto status_node = doc.node("Status");
            status_node.attr("fail", fails.size());
            status_node.attr("ok", ok_count);
            for (auto fp: fails)
            {
                auto fail_node = status_node.node("Fail");
                fail_node.attr("path", fp);
            }
            MSS(fails.empty(), log::error() << "Some commands failed" << std::endl);
        }
        else
        {
            MSS(!options_.input_filepaths.empty());

            onto::Filepath__Node filepath__node;

            onto::Node root{onto::Type::Root};

            using Filepaths = std::set<std::string>;
            Filepaths filepaths;
            for (const auto &filepath: options_.input_filepaths)
            {
                root.childs.emplace_back(onto::Type::Link);
                auto &link = root.childs.back();
                link.metadata.input.linkpath = filepath;
                filepaths.insert(filepath);
            }

            //Load all the nodes with a metadata.input.linkpath from file
            while (filepaths.size() != filepath__node.size())
            {
                for (const auto &filepath: filepaths)
                    if (filepath__node.count(filepath) == 0)
                    {
                        auto &node = filepath__node[filepath];
                        MSS(input::load_from_file(node, filepath));

                        auto insert_into_filepaths = [&](const std::string &filepath){
                            filepaths.insert(filepath);
                        };
                        node.each_linkpath(insert_into_filepaths);
                    }
            }

            //Aggregate metadata
            root.aggregate_metadata(nullptr);
            for (auto &[_, node]: filepath__node)
                node.aggregate_metadata(nullptr);

            //Merge linkpaths until stable
            while (true)
            {
                unsigned int count = 0u;
                root.merge_linkpaths(count, filepath__node);
                for (auto &[_, node]: filepath__node)
                    node.merge_linkpaths(count, filepath__node);
                if (count == 0)
                    break;
            }

            MSS(root.aggregate_linkpaths(filepath__node));
            for (auto &[_, node]: filepath__node)
            {
                MSS(node.aggregate_linkpaths(filepath__node));
            }

            std::cout << root;
            for (const auto &[filepath, node]: filepath__node)
            {
                std::cout << std::endl << filepath << std::endl;
                std::cout << node;
            }

            if (options_.operation_opt)
            {
                switch (*options_.operation_opt)
                {
                    case Operation::Update:
                        for (const auto &[filepath, node]: filepath__node)
                        {
                            std::ofstream fo{filepath};
                            onto::Node::StreamConfig stream_config;
                            stream_config.mode = onto::Node::StreamConfig::Original;
                            stream_config.include_aggregates = options_.include_aggregates;
                            node.stream(fo, 0, stream_config);
                        }
                        break;
                    case Operation::Export:
                        {
                            MSS(!options_.output_filepath.empty(), log::error() << "Export requires an output filepath" << std::endl;);
                            std::ofstream fo{options_.output_filepath};
                            onto::Node::StreamConfig stream_config;
                            stream_config.mode = onto::Node::StreamConfig::Export;
                            root.stream(fo, 0, stream_config);
                        }
                        break;
                }
            }
        }

        MSS_END();
    }

}
