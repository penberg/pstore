require 'tempfile'

class PStoreWorld
    def run(command)
        stderr_file = Tempfile.new('pstore-world')
        stderr_file.close
        @stdout = `#{command} 2> #{stderr_file.path}`
        @stderr = IO.read(stderr_file.path)
        @exitstatus = $?.exitstatus
    end
end

World do
    PStoreWorld.new
end
