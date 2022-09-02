#ifndef HEADER_dpn_Library_hpp_ALREADY_INCLUDED
#define HEADER_dpn_Library_hpp_ALREADY_INCLUDED

#include <dpn/config/Config.hpp>
#include <dpn/File.hpp>
#include <dpn/List.hpp>

#include <gubg/std/filesystem.hpp>

#include <map>
#include <ostream>

namespace dpn { 

	class Library
	{
	public:
		Library(const config::Config &config): config_(config) {}

		void clear();

		bool add_file(const std::filesystem::path &);

		bool resolve();

		void print_debug(std::ostream &) const;

		bool get(List &, meta::Status) const;
		bool get_due(List &) const;

		template <typename Ftor>
		void each_file(Ftor &&ftor){for (const auto &[_,file]: fp__file_) ftor(file);}

	private:
		bool resolve_include_(std::filesystem::path &fp, std::string incl, const std::filesystem::path &context_fp) const;

		const config::Config &config_;

		std::map<std::filesystem::path, File> fp__file_;
	};

} 

#endif