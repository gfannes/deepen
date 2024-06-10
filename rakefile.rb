require_relative(File.join(ENV["gubg"], "gubg.build/load"))
require('fileutils')

task :default do
    sh "rake -T"
end

desc "Install"
task :install => :build do
    sh "cp dpn.app #{ENV["gubg"]}/bin/dpn"

    here_dir = File.dirname(__FILE__)
    bin_dir = File.join(ENV['HOME'], '.local/bin')
    FileUtils.mkdir_p(bin_dir)
    name = 'dpnlsp'
    fp = File.join(bin_dir, name)
    File.open(fp, 'w') do |fo|
        fo.puts("#!/usr/bin/env ruby")
        fo.puts("$LOAD_PATH << '#{File.join(here_dir, 'src')}'")
        fo.puts("require('dpn/lsp/main')")
        fo.puts("Dpn::Lsp.main()")
    end
    FileUtils.chmod(0755, fp)
end

def cooker(&block)
    require("gubg/build/Cooker")
    c = Gubg::Build::Cooker.new

    toolchain = {linux: :gcc, windows: :msvc, macos: :clang}[Gubg.os()]
    c.toolchain(toolchain)

    cpp_version = {gcc: "20", msvc: :latest, clang: "17"}[toolchain]
    c.option("c++.std", cpp_version)
    c.option("fail_on_warning", "return")
    c.option("debug_symbols", "true")

    case Gubg.os()
    when :linux
    when :macos
        c.option("target", "x86_64-apple-macos11.3")
    when :windows
    end

    block.yield(c)
end

desc "Create compile_commands.json database"
task :cc => :prepare do
    cooker do |c|
        mode = :debug
        # mode = :release
        c.option(mode)
        c.generate(:ninja, "dpn/app")
        c.ninja_compdb()
    end
end

task :prepare do
    if !File.exist?("gubg")
        gubg = ENV["gubg"]
        puts("Creating symlink to #{gubg}")
        raise("Error: you have to set the `gubg` env var") unless gubg
        File.symlink(gubg, "gubg")
    end
end

desc "Build"
task :build => :prepare do
    cooker do |c|
        mode = :debug
        # mode = :release
        c.option(mode)
        c.generate(:ninja, "dpn/app")
        c.ninja()
    end
end

desc "Clean"
task :clean do
    FileUtils.rm_rf(".cook")
end


desc "Run all tests"
task :test => [:ut, :e2e]

desc "Run unit tests, filter is colon-separated selection filter"
task :ut, [:filter] => :prepare do |t,args|
    filter = (args[:filter] || "ut").split(":").map{|e|"[#{e}]"}*""

    cooker do |c|
        mode = :debug
        # mode = :release
        c.option(mode)
        c.generate(:ninja, "dpn/ut")
        c.ninja()
        args = %w[-d yes -a]+filter
        c.run(*args)
    end
end

desc "Run end-to-end tests"
task :e2e => :build do
    sh "./dpn.app -h"
    sh "./dpn.app -i README.md"
end
