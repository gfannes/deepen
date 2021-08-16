task :default do
    sh "rake -T"
end


desc "Install"
task :install => :build do
    sh "sudo cp dpn.app /usr/local/bin/dpn"
end

desc "Build"
task :build do
    mode = :debug
    # mode = :release
    sh "cook -g ninja -T c++.std=2a -T #{mode} -O .cook/#{mode} dpn/app"
    sh "ninja -v"
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
