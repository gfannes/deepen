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

            onto::AbsFilepath__Node abs_filepath__node;

            onto::Node root{onto::Type::Root};

            using AbsFilepaths = std::set<std::filesystem::path>;
            AbsFilepaths abs_filepaths;
            for (const auto &filepath: options_.input_filepaths)
            {
                root.childs.emplace_back(onto::Type::Link);
                auto &link = root.childs.back();
                link.metadata.input.linkpath_rel = filepath;
                const auto filepath_abs = std::filesystem::absolute(filepath);
                link.metadata.input.linkpath_abs = filepath_abs;
                abs_filepaths.insert(filepath_abs);
            }

            //Load all the nodes with a metadata.input.linkpath from file
            while (abs_filepaths.size() != abs_filepath__node.size())
            {
                const auto copy_abs_filepaths = abs_filepaths;
                for (const auto &abs_filepath: copy_abs_filepaths)
                    if (abs_filepath__node.count(abs_filepath) == 0)
                    {
                        auto &node = abs_filepath__node[abs_filepath];
                        log::os(1) << "Loading `" << abs_filepath << "`" << std::endl;
                        MSS(input::load_from_file(node, abs_filepath));

                        auto insert_into_abs_filepaths = [&](const std::string &new_abs_filepath){
                            abs_filepaths.insert(new_abs_filepath);
                        };
                        node.each_abs_linkpath(insert_into_abs_filepaths);
                    }
            }

            //Aggregate metadata
            root.aggregate_metadata(nullptr);
            for (auto &[_, node]: abs_filepath__node)
                node.aggregate_metadata(nullptr);

            //Merge linkpaths until stable
            while (true)
            {
                unsigned int count = 0u;
                root.merge_linkpaths(count, abs_filepath__node);
                for (auto &[_, node]: abs_filepath__node)
                    node.merge_linkpaths(count, abs_filepath__node);
                if (count == 0)
                    break;
            }

            MSS(root.aggregate_linkpaths(abs_filepath__node));
            for (auto &[_, node]: abs_filepath__node)
            {
                MSS(node.aggregate_linkpaths(abs_filepath__node));
            }

            log::os(2) << root;
            for (const auto &[abs_filepath, node]: abs_filepath__node)
            {
                log::os(2) << std::endl << abs_filepath << std::endl;
                log::os(2) << node;
            }

            if (options_.operation_opt)
            {
                switch (*options_.operation_opt)
                {
                    case Operation::Update:
                        for (const auto &[abs_filepath, node]: abs_filepath__node)
                        {
                            std::ofstream fo{abs_filepath};
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
