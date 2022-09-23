#include <dpn/App.hpp>
#include <dpn/log.hpp>
#include <dpn/input/from_file.hpp>
#include <dpn/plan/Planner.hpp>

#include <gubg/naft/Document.hpp>
#include <gubg/std/filesystem.hpp>
#include <gubg/mss.hpp>

#include <termcolor/termcolor.hpp>

#include <fstream>
#include <list>
#include <algorithm>
#include <iomanip>
#include <numeric>

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
        MSS_BEGIN(bool);

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

            case Verb::Plan:                    MSS(plan_(), log::error() << "Could not run plan" << std::endl); break;

            case Verb::Show:
            MSS(!!options_.show_opt);
            switch (*options_.show_opt)
            {
                case Show::Inbox:       MSS(show_list_(meta::Status::Inbox),      log::error() << "Could not work on Inbox items" << std::endl); break;
                case Show::Actionable:  MSS(show_list_(meta::Status::Actionable), log::error() << "Could not work on Actionable items" << std::endl); break;
                case Show::Forwarded:   MSS(show_list_(meta::Status::Forwarded),  log::error() << "Could not work on Forwarded items" << std::endl); break;
                case Show::WIP:         MSS(show_list_(meta::Status::WIP),        log::error() << "Could not work on WIP items" << std::endl); break;
                case Show::Done:        MSS(show_list_(meta::Status::Done),       log::error() << "Could not work on Done items" << std::endl); break;
                case Show::DueDate:     MSS(show_list_due_(),                     log::error() << "Could not work on items with Duedate" << std::endl); break;
                case Show::Features:    MSS(show_features_(),                     log::error() << "Could not show Features" << std::endl); break;
                case Show::Todo:        MSS(show_todo_(),                         log::error() << "Could not show Todo" << std::endl); break;
                case Show::KeyValues:   MSS(show_key_values_(false),              log::error() << "Could not show KeyValues" << std::endl); break;
                case Show::KeyValues_v: MSS(show_key_values_(true),               log::error() << "Could not show KeyValues_v" << std::endl); break;
                case Show::Debug:       MSS(print_debug_(),                       log::error() << "Could not show Debug" << std::endl); break;
            }
            break;

            default:                            MSS(false,          log::error() << "Unknown verb " << (int)verb << std::endl); break;
        }

        MSS_END();
    }

    //Privates
    bool App::plan_()
    {
        MSS_BEGIN(bool);

        MSS(load_ontology_(), log::error() << "Could not load the ontology" << std::endl);

        List nodes;
        Id__Id part_of, after;
        Id__DepIds requires;
        const Library::Filter filter = {.incl_tags = options_.incl_tags, .excl_tags = options_.excl_tags, };
        MSS(library_.get_graph(nodes, part_of, after, requires, filter), log::error() << "Could not get graph" << std::endl);

        plan::Planner planner;
        MSS(planner.setup_tasks(nodes, part_of, after, requires), log::error() << "Could not setup the tasks in the planner" << std::endl);
        
        if (options_.resources_fp)
        {
            const auto &fp = *options_.resources_fp;
            MSS(planner.load_resources(fp), log::error() << "Could not load resources from " << fp << std::endl);
        }
        
        plan::Plan plan;
        MSS(planner(plan), log::error() << "Could not create a plan" << std::endl);
        std::cout << plan << std::endl;

        MSS_END();
    }

    bool App::show_items_(List &list, Sort sort, bool reverse, const Library::Filter &filter) const
    {
        MSS_BEGIN(bool);
        
        list.sort(sort);

        using Level = int;
        constexpr Level Total = -1;
        constexpr Level Item = 0;

        std::vector<std::size_t> ixs(list.items.size());
        std::iota(ixs.begin(), ixs.end(), 0);
        if (reverse)
            std::reverse(ixs.begin(), ixs.end());

        auto each_row = [&](auto lambda){
            for (const auto ix: ixs)
            {
                const auto &item = list.items[ix];
                lambda(Item, ix, item.filtered_effort(), item.node().my_effort, to_string(item.path), item.text(), item.fp.string());
                if (options_.all_details || options_.details.count(ix))
                {
                    auto show_details = [&](const auto &node, const auto &path){
                        if (!filter(node))
                            return;
                        if (path.empty())
                            return;
                        if (node.text.empty() && node.filtered_effort.total == 0)
                            return;
                        lambda(path.size(), -1, node.filtered_effort, node.my_effort, std::string(2*path.size(), ' ')+node.text, "", library_.get_fp(node).value_or(""));
                    };
                    library_.each_node(item.node(), show_details, Direction::Push);
                }
            }
            lambda(Total, list.items.size(), list.effort, meta::Effort{}, "TOTAL", "", "");
        };

        std::size_t max_ix_w = 0, max_agg_effort_w = 0, max_my_todo_w = 0, max_path_w = 0, max_text_w = 0, max_fp_w = 0, max_path_text_w = 0;
        auto update_max_w = [&](Level, int ix, const meta::Effort &agg_effort, const meta::Effort &my_effort, const std::string &text, const std::string &path, const std::string &fp){
            if (ix >= 0)
                max_ix_w = std::max(max_ix_w, std::to_string(ix).size());
            max_agg_effort_w = std::max(max_agg_effort_w, agg_effort.str().size());
            max_my_todo_w = std::max(max_my_todo_w, meta::Effort::to_dsl(my_effort.todo()).size());
            const auto text_size = std::min<std::size_t>(text.size(), 150u);
            max_text_w = std::max(max_text_w, text_size);
            max_path_w = std::max(max_path_w, path.size());
            max_path_text_w = std::max(max_path_text_w, text_size+path.size());
        };
        each_row(update_max_w);

        std::string prev_fp;
        auto print_color = [&](Level level, int ix, const meta::Effort &agg_effort, const meta::Effort &my_effort, const std::string &path, const std::string &text, const std::string &fp){
            auto ix_color = termcolor::white<char>;
            auto effort_color = termcolor::white<char>;
            auto path_color = termcolor::white<char>;
            auto text_color = termcolor::white<char>;
            auto fp_color = termcolor::white<char>;

            switch (level)
            {
                case Total:
                ix_color = termcolor::magenta<char>;
                effort_color = termcolor::magenta<char>;
                path_color = termcolor::magenta<char>;
                text_color = termcolor::magenta<char>;
                break;

                case Item:
                ix_color = termcolor::white<char>;
                effort_color = termcolor::blue<char>;
                path_color = termcolor::green<char>;
                text_color = termcolor::cyan<char>;
                fp_color = termcolor::white<char>;
                break;

                default:
                effort_color = termcolor::cyan<char>;
                switch (level)
                {
                    case 1: path_color = termcolor::color<250, 250, 0, char>; break;
                    case 2: path_color = termcolor::color<200, 150, 30, char>; break;
                    case 3: path_color = termcolor::color<160, 50, 60, char>; break;
                    case 4: path_color = termcolor::color<130, 0, 90, char>; break;
                    default: path_color = termcolor::color<100, 0, 120, char>; break;
                }
                break;
            }

            std::cout << ix_color << std::setw(max_ix_w) << std::left;
            if (ix >= 0)
                std::cout << ix;
            else
                std::cout << "";
            std::cout << termcolor::reset;

            std::cout << ' ' << effort_color << std::setw(max_agg_effort_w) << std::left;
            if (level > Item && (agg_effort.total == 0 || agg_effort == my_effort))
                std::cout << "";
            else
                std::cout << agg_effort;
            std::cout << termcolor::reset;

            std::cout << ' ' << effort_color << std::setw(max_my_todo_w) << std::left;
            if (my_effort.todo() > 0)
                std::cout << meta::Effort::to_dsl(my_effort.todo());
            else
                std::cout << "";
            std::cout << termcolor::reset;

            std::cout << ' ' << path_color                                                         << path << termcolor::reset;
            std::cout << ' ' << text_color << std::setw(max_path_text_w-path.size()) << std::left << text << termcolor::reset;
            std::cout << ' ' << fp_color   << std::setw(max_fp_w)                    << std::left << (fp != prev_fp ? fp : "") << termcolor::reset;
            std::cout << std::endl;

            prev_fp = fp;
        };

        auto print_tab = [&](Level, int ix, const meta::Effort &agg_effort, const meta::Effort &my_effort, const std::string &path, const std::string &text, const std::string &fp){
            std::cout << ix << '\t' << agg_effort << '\t' << my_effort << '\t' << path << '\t' << text << '\t' << fp << std::endl;
        };

        if (options_.color_output)
        {
            std::cout << termcolor::colorize;
            each_row(print_color);
        }
        else
        {
            each_row(print_tab);
        }

        MSS_END();
    }

    bool App::show_list_(meta::Status status)
    {
        MSS_BEGIN(bool);

        MSS(load_ontology_(), log::error() << "Could not load the ontology" << std::endl);

        List list;
        const Library::Filter filter = {.incl_tags = options_.incl_tags, .excl_tags = options_.excl_tags, .status = status, .moscow = options_.moscow};
        MSS(library_.get(list, filter));

        MSS(show_items_(list, options_.sort.value_or(Sort::Rice), options_.reverse, filter));

        if (options_.output_filepath)
            MSS(library_.export_mindmap(to_string(status), list, filter, *options_.output_filepath));

        MSS_END();
    }

    bool App::show_list_due_()
    {
        MSS_BEGIN(bool);

        MSS(load_ontology_(), log::error() << "Could not load the ontology" << std::endl);

        const Library::Filter filter = {.incl_tags = options_.incl_tags, .excl_tags = options_.excl_tags, .moscow = options_.moscow};

        List list;
        MSS(library_.get_due(list, filter));

        MSS(show_items_(list, options_.sort.value_or(Sort::DueDate), options_.reverse, filter));

        if (options_.output_filepath)
            MSS(library_.export_mindmap("DueDate", list, filter, *options_.output_filepath));
        
        MSS_END();
    }

    bool App::show_features_()
    {
        MSS_BEGIN(bool);

        MSS(load_ontology_(), log::error() << "Could not load the ontology" << std::endl);

        const Library::Filter filter = {.incl_tags = options_.incl_tags, .excl_tags = options_.excl_tags, .moscow = options_.moscow};

        List list;
        MSS(library_.get_features(list, filter));

        MSS(show_items_(list, options_.sort.value_or(Sort::No), options_.reverse, filter));

        if (options_.output_filepath)
        {
            const std::filesystem::path fp = *options_.output_filepath;
            const auto ext = fp.extension().string();
            if (ext == ".mm")
                MSS(library_.export_mindmap("Features", list, filter, fp));
            if (ext == ".xml")
            {
                if (false)
                {
                    List list;
                    Id__DepIds id__dep_ids;
                    MSS(library_.get_nodes_links(list, id__dep_ids, filter), log::error() << "Could not get nodes and links" << std::endl);
                    MSS(library_.export_msproj2(list, id__dep_ids, fp));
                }
                else
                {
                    MSS(library_.export_msproj(list, filter, fp));
                }
            }
        }
        
        MSS_END();
    }

    bool App::show_todo_()
    {
        MSS_BEGIN(bool);

        MSS(load_ontology_(), log::error() << "Could not load the ontology" << std::endl);

        const Library::Filter filter = {.incl_tags = options_.incl_tags, .excl_tags = options_.excl_tags, .moscow = options_.moscow};

        List list;
        MSS(library_.get_todo(list, filter));

        MSS(show_items_(list, options_.sort.value_or(Sort::No), options_.reverse, filter));
        
        MSS_END();
    }

    bool App::print_debug_()
    {
        MSS_BEGIN(bool);

        MSS(load_ontology_(), log::error() << "Could not load the ontology" << std::endl);

        const Library::Filter filter = {.incl_tags = options_.incl_tags, .excl_tags = options_.excl_tags, .moscow = options_.moscow};

        library_.print_debug(std::cout, filter);

        MSS_END();
    }

    bool App::show_key_values_(bool verbose)
    {
        MSS_BEGIN(bool);

        MSS(load_ontology_(), log::error() << "Could not load the ontology" << std::endl);

        TagSets tag_sets;
        using KV = std::pair<std::string, std::string>;
        std::map<KV, std::set<std::filesystem::path>> kv__fps;
        library_.each_file([&](const auto &file){
            auto collect = [&](const auto &node, const auto &path){
                if (node.has_matching_tags(options_.incl_tags, true) && !node.has_matching_tags(options_.excl_tags, false))
                {
                    for (const auto &kv: node.my_tags)
                    {
                        tag_sets[kv.first].insert(kv.second);
                        kv__fps[kv].insert(file.fp);
                    }
                }
            };
            file.each_node(collect, Direction::Push);
        });

        std::cout << termcolor::colorize;
        for (const auto &[key,values]: tag_sets)
        {
            std::cout << termcolor::yellow << key << termcolor::reset << std::endl;
            for (const auto &value: values)
            {
                std::cout << '\t' << termcolor::green << value << termcolor::reset << std::endl;
                if (verbose)
                {
                    for (const auto &fp: kv__fps[KV{key, value}])
                        std::cout << "\t\t" << termcolor::blue << fp << termcolor::reset << std::endl;
                }
            }
        }

        MSS_END();
    }

    bool App::list_()
    {
        MSS_BEGIN(bool);

        MSS(load_ontology_(), log::error() << "Could not load the ontology" << std::endl);

        onto::Node::StreamConfig stream_config;
        stream_config.detailed = options_.detailed_;
        stream_config.mode = onto::Node::StreamConfig::List;
        stream_config.abs_filepath__node = &abs_filepath__node_;
        if (!options_.tags_.empty())
        {
            std::string tmp = "@"; tmp += options_.tags_.front();
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
        stream_config.detailed = options_.detailed_;
        stream_config.mode = onto::Node::StreamConfig::Export;
        stream_config.abs_filepath__node = &abs_filepath__node_;
        if (!options_.tags_.empty())
        {
            std::string tmp = "@"; tmp += options_.tags_.front();
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
        for (std::string filepath: options_.input_filepaths)
        {
            auto &link = root_.childs.emplace_back(onto::Type::Link);
            MSS(config_.substitute_names(filepath));
            link.metadata.input.linkpath_rel = filepath;
            const auto filepath_abs = std::filesystem::absolute(filepath);
            link.metadata.input.linkpath_abs = filepath_abs;
            abs_filepaths.insert(filepath_abs);

            MSS(library_.add_file(filepath_abs, true), log::error() << "Could not add " << filepath_abs << " to library" << std::endl);
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
            for (const auto &tag: options_.tags_)
            {
                std::string tmp = "@"; tmp += tag;
                gubg::Strange strange{tmp};
                MSS(item.parse(strange), log::error() << "Tag `" << tag << "` has incorrect format" << std::endl);

                ns__values[item.key].insert(item.value);
            }

            MSS_END();
        }

    }
