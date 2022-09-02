#ifndef HEADER_dpn_App_hpp_ALREADY_INCLUDED
#define HEADER_dpn_App_hpp_ALREADY_INCLUDED

#include <dpn/Options.hpp>
#include <dpn/config/Config.hpp>
#include <dpn/Library.hpp>
#include <dpn/onto/Node.hpp>

namespace dpn { 

    class App
    {
    public:
        App(const Options &options): options_(options) {}

        bool load_config();

        bool run();

    private:
        bool list_();
        bool print_debug_();
        bool show_list_(meta::Status);
        bool show_list_due_();
        bool show_projects_();
        bool show_todo_();
        void show_items_(const List &) const;
        bool update_(bool with_aggregates);
        bool export_();
        bool run_command_();

        Options options_;
        config::Config config_;

        bool load_tags_(metadata::Ns__Values &) const;

        bool load_ontology_();
        Library library_{config_};
        onto::AbsFilepath__Node abs_filepath__node_;
        onto::Node root_{onto::Type::Root};
    };

}

#endif
