require_relative(File.join(ENV["gubg"], "gubg.build/load"))

task :default do
    sh "rake -T"
end

desc "Install"
task :install => :build do
    sh "cp dpn.app #{ENV["gubg"]}/bin/dpn"
end

def cooker(&block)
    require("gubg/build/Cooker")
    c = GUBG::Build::Cooker.new

    toolchain = {linux: :gcc, windows: :msvc, macos: :clang}[GUBG.os()]
    c.toolchain(toolchain)

    cpp_version = {gcc: "17", msvc: :latest, clang: "17"}[toolchain]
    c.option("c++.std", cpp_version)
    c.option("fail_on_warning", "return")
    c.option("debug_symbols", "true")

    case GUBG.os()
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
        mode = :release
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
