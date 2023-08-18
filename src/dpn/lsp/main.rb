module Dpn
    module Lsp
        def self.main()
            File.open('/tmp/dpnlsp.log', 'w') do |fo|
                fo.puts(Time.now())
                ARGF.each do |line|
                    fo.puts(line)
                end
            end
        end
    end
end
