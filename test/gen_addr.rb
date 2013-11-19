
if ARGV.length > 0 then
   filename = ARGV[0]
else
   filename = "addr.dat"
end

open( filename, "wb" ) do |file|
   (0...0x100).to_a.each do |i|
      file.write [i].pack("I1")
   end
end
puts "done. saved as #{filename}."