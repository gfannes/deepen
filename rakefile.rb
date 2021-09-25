require("gubg/shared")

task :default do
    sh "rake -T"
end


desc "Install"
task :install => :build do
    sh "cp dpn.app #{ENV["gubg"]}/bin/dpn"
end

desc "Create compile_commands.json database"
task :cc do
    sh "cook -g ninja"
    sh "ninja -t compdb > compile_commands.json"
end

desc "Build"
task :build do
    mode = :debug
    mode = :release
    toolchain = {linux: :gcc, windows: :msvc, macos: :clang}[GUBG.os()]
    cpp_version = {gcc: "2a", msvc: :latest, clang: "17"}[toolchain]
    target = {macos: "-T target=x86_64-apple-macos11.3"}[GUBG.os()]
    sh "cook -g ninja -t #{toolchain} -T c++.std=#{cpp_version} -T #{mode} #{target} -O .cook/#{mode} dpn/app"
    sh "ninja -v"
end

desc "Clean"
task :clean do
    FileUtils.rm_rf(".cook")
end


desc "Run all tests"
task :test => [:ut, :e2e]

desc "Run unit tests, filter is colon-separated selection filter"
task :ut, [:filter] do |t,args|
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
