# -*- mode:ruby -*-

if ARGV.length >= 2 then
   filename = ARGV[0]
   multiplier = ARGV[1].to_i
else
   puts "usage: #{$0} <output file> <multiplier>"
   exit 1
end

open( filename, "wb" ) do |file|
   (0...0x100).to_a.each do |i|
      file.write [i*multiplier].pack("I1")
   end
end
puts "done. saved as #{filename}."
