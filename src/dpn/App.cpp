#include <dpn/App.hpp>
#include <dpn/log.hpp>
#include <dpn/input/from_file.hpp>

#include <gubg/naft/Document.hpp>
#include <gubg/std/filesystem.hpp>
#include <gubg/mss.hpp>

#include <fstream>
#include <list>

namespace dpn { 

    bool App::load_config()
    {
        MSS_BEGIN(bool);

        std::string filepath;
        MSS(config::get_default_config_filepath(filepath));

        if (!config_.load_from_file(filepath))
            log::warning() << "Could not load the configuration from file, using an empty configuration" << std::endl;

        MSS_END();
    }

    bool App::run()
    {
        MSS_BEGIN(bool, "");

        // Use default input_filepaths from config if none were specified
        if (options_.input_filepaths.empty())
            for (const auto &fp: config_.default_inputs)
                options_.input_filepaths.push_back(fp);

        MSS(!!options_.verb_opt, log::error() << "No verb was specified" << std::endl);
        const auto verb = *options_.verb_opt;

        switch (verb)
        {
            case Verb::Help:                    MSS(false,          log::internal_error() << "Printing help should be handled in Options" << std::endl); break;

            case Verb::UpdateWithoutAggregates: MSS(update_(false), log::error() << "Could not update" << std::endl); break;
            case Verb::UpdateWithAggregates:    MSS(update_(true) , log::error() << "Could not update" << std::endl); break;

            case Verb::Export:                  MSS(export_(),      log::error() << "Could not export" << std::endl); break;

            case Verb::List:                    MSS(list_(),        log::error() << "Could not list" << std::endl); break;

            case Verb::Run:                     MSS(run_command_(), log::error() << "Could not run command" << std::endl); break;

            case Verb::Print:                   MSS(print_(),       log::error() << "Could not print" << std::endl); break;

            case Verb::Inbox:                   MSS(show_list_(meta::State::Inbox),       log::error() << "Could not work on Inbox items" << std::endl); break;
            case Verb::Actionable:              MSS(show_list_(meta::State::Actionable),  log::error() << "Could not work on Actionable items" << std::endl); break;
            case Verb::Forwarded:               MSS(show_list_(meta::State::Forwarded),   log::error() << "Could not work on Forwarded items" << std::endl); break;
            case Verb::WIP:                     MSS(show_list_(meta::State::WIP),         log::error() << "Could not work on WIP items" << std::endl); break;
            case Verb::Duedate:                 MSS(show_list_due_(),                     log::error() << "Could not work on items with Duedate" << std::endl); break;

            default:                            MSS(false,          log::error() << "Unknown verb " << (int)verb << std::endl); break;
        }

        MSS_END();
    }

    //Privates
    void App::show_items_(const List &list) const
    {
        std::cout << "{" << std::endl;
        for (auto item: list.items)
        {
            std::cout << "  [" << item.text << "](fp:" << item.fp << ")";
            if (item.state)
                std::cout << "(state:" << *item.state << ")";
            std::cout << "(prio:" << item.prio << ")(cost:" << item.cost << ")(rice:" << item.rice() << ")";
            if (item.yyyymmdd)
                std::cout << "(due:" << *item.yyyymmdd << ")";
            std::cout << std::endl;
        }
        std::cout << "}" << std::endl;
    }

    bool App::show_list_(meta::State state)
    {
        MSS_BEGIN(bool);

        MSS(load_ontology_(), log::error() << "Could not load the ontology" << std::endl);

        List list;
        MSS(library_.get(list, state));
        list.sort_on_rice();
        std::cout << "[" << state << "](size:" << list.items.size() << ")" << std::endl;
        show_items_(list);

        MSS_END();
    }

    bool App::show_list_due_()
    {
        MSS_BEGIN(bool);

        MSS(load_ontology_(), log::error() << "Could not load the ontology" << std::endl);

        List list;
        MSS(library_.get_due(list));
        list.sort_on_duedate();
        std::cout << "[Duedate](size:" << list.items.size() << ")" << std::endl;
        show_items_(list);
        
        MSS_END();
    }

    bool App::print_()
    {
        MSS_BEGIN(bool);

        MSS(load_ontology_(), log::error() << "Could not load the ontology" << std::endl);

        library_.print(std::cout);

        MSS_END();
    }


    bool App::list_()
    {
        MSS_BEGIN(bool);

        MSS(load_ontology_(), log::error() << "Could not load the ontology" << std::endl);

        onto::Node::StreamConfig stream_config;
        stream_config.detailed = options_.detailed;
        stream_config.mode = onto::Node::StreamConfig::List;
        stream_config.abs_filepath__node = &abs_filepath__node_;
        if (!options_.tags.empty())
        {
            std::string tmp = "@"; tmp += options_.tags.front();
            gubg::Strange strange{tmp};
            stream_config.filter.emplace();
            MSS(stream_config.filter->parse(strange), log::error() << "Tag has incorrect format" << std::endl);
        }
        root_.stream(std::cout, 0, stream_config);

        MSS_END();
    }

    bool App::export_()
    {
        MSS_BEGIN(bool);

        MSS(load_ontology_(), log::error() << "Could not load the ontology" << std::endl);

        MSS(!!options_.output_filepath, log::error() << "Export requires an output filepath" << std::endl;);
        for (const auto &input_filepath: options_.input_filepaths)
        {
            MSS(*options_.output_filepath != input_filepath, log::error() << "The output filepath clashes with one of the input filepaths" << std::endl);
        }
        std::ofstream fo{*options_.output_filepath};

        if (options_.format)
        {
            root_.set_format(*options_.format);
            for (auto &[_, node]: abs_filepath__node_)
                node.set_format(*options_.format);
        }

        onto::Node::StreamConfig stream_config;
        stream_config.detailed = options_.detailed;
        stream_config.mode = onto::Node::StreamConfig::Export;
        stream_config.abs_filepath__node = &abs_filepath__node_;
        if (!options_.tags.empty())
        {
            std::string tmp = "@"; tmp += options_.tags.front();
            gubg::Strange strange{tmp};
            stream_config.filter.emplace();
            MSS(stream_config.filter->parse(strange), log::error() << "Tag has incorrect format" << std::endl);
        }
        root_.stream(fo, 0, stream_config);

        MSS_END();
    }

    bool App::update_(bool with_aggregates)
    {
        MSS_BEGIN(bool);

        MSS(load_ontology_(), log::error() << "Could not load the ontology" << std::endl);

        for (const auto &[abs_filepath, node]: abs_filepath__node_)
        {
            std::ofstream fo{abs_filepath};
            onto::Node::StreamConfig stream_config;
            stream_config.mode = onto::Node::StreamConfig::Original;
            stream_config.include_aggregates = with_aggregates;
            node.stream(fo, 0, stream_config);
        }

        MSS_END();
    }

    bool App::run_command_()
    {
        MSS_BEGIN(bool);

        const auto &command_args = options_.arguments;

        gubg::naft::Document doc{std::cout};
        unsigned int ok_count = 0;
        std::list<std::string> fails;
        for (const auto &root_path: config_.paths)
        {
            try
            {
                std::filesystem::current_path(root_path);
                auto run_node = doc.node("Run");
                run_node.attr("pwd", std::filesystem::current_path().string());
                {
                    auto command_node = run_node.node("Command");
                    for (const auto &arg: command_args)
                        command_node.attr("arg", arg);
                }
                run_node.text("\n");

                std::string command;
                for (const auto &arg: command_args)
                {
                    if (!command.empty())
                        command.push_back(' ');
                        //@todo: escape space
                    command += arg;
                }

                std::flush(std::cout);
                const auto rc = std::system(command.c_str());
                {
                    auto status_node = run_node.node("Status"); 
                    status_node.attr("rc", rc);
                    if (rc == 0)
                        ++ok_count;
                    else
                        fails.push_back(root_path);
                }
            }
            catch (...)
            {
                log::error() << "Path `" << root_path << "` does not exist" << std::endl;
                fails.push_back(root_path);
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
        MSS_END();
    }

    bool App::load_ontology_()
    {
        MSS_BEGIN(bool);

        MSS(!options_.input_filepaths.empty(), log::error() << "Cannot load ontology without input filepaths" << std::endl);

        library_.clear();

        using AbsFilepaths = std::set<std::filesystem::path>;
        AbsFilepaths abs_filepaths;
        for (const auto &filepath: options_.input_filepaths)
        {
            auto &link = root_.childs.emplace_back(onto::Type::Link);
            link.metadata.input.linkpath_rel = config_.substitute_names(filepath);
            const auto filepath_abs = std::filesystem::absolute(filepath);
            link.metadata.input.linkpath_abs = filepath_abs;
            abs_filepaths.insert(filepath_abs);

            MSS(library_.add_file(filepath_abs), log::error() << "Could not add " << filepath_abs << " to library" << std::endl);
        }

        MSS(library_.resolve());

        //Load all the nodes with a metadata.input.linkpath from file
        while (abs_filepaths.size() != abs_filepath__node_.size())
        {
            const auto copy_abs_filepaths = abs_filepaths;
            for (const auto &abs_filepath: copy_abs_filepaths)
                if (abs_filepath__node_.count(abs_filepath) == 0)
                {
                    auto &node = abs_filepath__node_[abs_filepath];
                    log::os(1) << "Loading `" << abs_filepath << "`" << std::endl;
                    MSS(input::load_from_file(node, abs_filepath, config_));

                    auto insert_into_abs_filepaths = [&](const auto &new_abs_filepath){
                        abs_filepaths.insert(new_abs_filepath);
                    };
                    node.each_abs_linkpath(insert_into_abs_filepaths);
                }
            }

            {
                metadata::Ns__Values ns__values;
                MSS(load_tags_(ns__values));

            //Aggregate metadata
                root_.aggregate_metadata(nullptr, ns__values);
                for (auto &[_, node]: abs_filepath__node_)
                    node.aggregate_metadata(nullptr, ns__values);
            }

        //Merge linkpaths until stable
            while (true)
            {
                unsigned int count = 0u;
                root_.merge_linkpaths(count, abs_filepath__node_);
                for (auto &[_, node]: abs_filepath__node_)
                    node.merge_linkpaths(count, abs_filepath__node_);
                if (count == 0)
                    break;
            }

            MSS(root_.aggregate_linkpaths(abs_filepath__node_));
            for (auto &[_, node]: abs_filepath__node_)
            {
                MSS(node.aggregate_linkpaths(abs_filepath__node_));
            }

            log::os(2) << root_;
            for (const auto &[abs_filepath, node]: abs_filepath__node_)
            {
                log::os(2) << std::endl << abs_filepath << std::endl;
                log::os(2) << node;
            }

            MSS_END();
        }

        bool App::load_tags_(metadata::Ns__Values &ns__values) const
        {
            MSS_BEGIN(bool);

            ns__values.clear();

            metadata::Item item;
            for (const auto &tag: options_.tags)
            {
                std::string tmp = "@"; tmp += tag;
                gubg::Strange strange{tmp};
                MSS(item.parse(strange), log::error() << "Tag `" << tag << "` has incorrect format" << std::endl);

                ns__values[item.key].insert(item.value);
            }

            MSS_END();
        }

    }
