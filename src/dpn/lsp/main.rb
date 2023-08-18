module Dpn
    module Lsp
        def self.main()
            File.open('/tmp/dpnlsp.log', 'w') do |fo|
                fo.puts("Hello from Dpn::Lsp")
            end
        end
    end
end
