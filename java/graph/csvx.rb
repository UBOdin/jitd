class String
  def from_csv
    ret = chomp.split(/,/)
    idx = 0;
    while idx < ret.length do
      if ret[idx][0] == "\""[0]
        while ret[idx][-1] != "\""[0]
          raise "Unterminated quote" if idx+1 >= ret.length
          ret[idx] = ret[idx]+","+ret[idx+1]
          ret.delete_at(idx+1)
        end
        ret[idx] = ret[idx].sub(/^"/, "").sub(/"$/, "")
      end
      idx += 1
    end
    ret
  end
end

class Array
  def from_csv
    self.map { |l| l.to_s.chomp.from_csv }
  end
end

class IO
  def from_csv(header = false)
    keys = readline.chomp.from_csv if header;
    map { |l| l.to_s.chomp.from_csv }.
      map { |a| if header then keys.zip(a).to_h else a end }
  end
end

class File
  def File.csv(f, header = false)
    File.open(f) {|io| io.from_csv(header) }
  end
end
