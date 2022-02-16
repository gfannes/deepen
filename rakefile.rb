require("gubg/shared")

task :default do
    sh "rake -T"
end


desc "Install"
task :install => :build do
    sh "cp dpn.app #{ENV["gubg"]}/bin/dpn"
end

desc "Create compile_commands.json database"
task :cc => :prepare do
    sh "cook -g ninja"
    sh "ninja -t compdb > compile_commands.json"
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
    mode = :debug
    mode = :release
    toolchain = {linux: :gcc, windows: :msvc, macos: :clang}[GUBG.os()]
    cpp_version = {gcc: "2a", msvc: :latest, clang: "17"}[toolchain]
    target = {macos: "-T target=x86_64-apple-macos11.3"}[GUBG.os()]
    compiler, linker = nil, nil
    if true
        compiler = "-T compiler=gcc-9"
        linker = "-T linker=g++-9"
    end
    sh "cook -g ninja -t #{toolchain} -T c++.std=#{cpp_version} -T #{mode} #{compiler} #{linker} -O .cook/#{mode} dpn/app"
    sh "ninja -v"
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

    mode = :debug
    # mode = :release
    sh "cook -g ninja -T c++.std=2a -T #{mode} -O .cook/#{mode} dpn/ut"
    sh "ninja -v"

    cmd = "./dpn.ut -d yes -a".split(" ") << filter
    sh(*cmd)
end

desc "Run end-to-end tests"
task :e2e => :build do
    sh "./dpn.app -h"
    sh "./dpn.app -i README.md"
end
