
$plot_terminal = "aqua"
$plot_suffix = nil;
$plot_auto_open = false;
$current_plot = nil;

def plot_output(output, settings = nil)
  case output
    when :aqua then $plot_terminal = "aqua"; $plot_suffix = nil;
    when :pdf then $plot_terminal = "pdf"; $plot_suffix = ".pdf"
    when :png then $plot_terminal = "png"; $plot_suffix = ".png"
  end
  $plot_terminal += " "+settings unless settings.nil?
end

$pretty_styles = [
  {  :lt => "rgb \"#A00000\"", 
     :lw => 2,
     :pt => 1
  },
  {  :lt => "rgb \"#00A000\"", 
     :lw => 2,
     :pt => 6
  },
  {  :lt => "rgb \"#5060D0\"", 
     :lw => 2,
     :pt => 2
  },
  {  :lt => "rgb \"#F25900\"", 
     :lw => 2,
     :pt => 9
  }
];

def pretty_style(idx, opts = {})
  opts = opts.clone;
  $pretty_styles[idx].each { |k, v| opts[k] = v unless opts.has_key? k }
  opts.map { |kv| kv.to_a.join(" ") unless kv[1].nil? }.compact.join(" ")
end

def pretty_plot(plot, opts = {})
  # plot based on Brighten Godfrey's blog post:
  # http://youinfinitesnake.blogspot.com/2011/02/attractive-scientific-plots-with.html
  
  plot.terminal [
    "pdf",
    "font \"#{opts.fetch(:fontface, "Times-Roman")},#{opts.fetch(:fontsize, 10)}\"",
    "linewidth 4 #{opts.fetch(:rounded, true) ? "rounded" : ""}",
    "fontscale #{opts.fetch(:fontscale, 1.0)}",
    "size #{opts.fetch(:sizex, 5)}in,#{opts.fetch(:sizey, 3)}in"
  ].join(" ")
  
  # Line style for axes
  plot.style "line 80 lt rgb \"#808080\""

  # Line style for grid
  plot.style "line 81 lt 0"  # dashed
  plot.style "line 81 lt rgb \"#808080\""  # grey

  plot.grid "back linestyle 81"
  plot.border "3 back linestyle 80" # Remove border on top and right.  These
                                  # borders are useless and make it harder
                                  # to see plotted lines near the border.
    # Also, put it in grey; no need for so much emphasis on a border.
  plot.xtics "nomirror"
  plot.ytics "nomirror"

  if(opts.fetch(:logx, false)) then
    plot.logscal "x"
    plot.mxtics "10"    # Makes logscale look good.
  end
  if(opts.fetch(:logy, false)) then
    plot.logscal "y"
    plot.mytics "10"    # Makes logscale look good.
  end
  
  # Line styles: try to pick pleasing colors, rather
  # than strictly primary colors or hard-to-see colors
  # like gnuplot's default yellow.  Make the lines thick
  # so they're easy to see in small plots in papers.
  $pretty_styles.each_index { |x| plot.style "line #{x+1} #{pretty_style(x)}" }

  plot.key "bottom right"
end

def auto_open_plots(new_val = true)
  $plot_auto_open = new_val;
end

def row_data(data)
  $current_plot.data << Gnuplot::DataSet.new(data.unzip) { |ds| yield ds }
end


def plot(args = {})
  task(args) do
    task_name = case args
      when Hash then args.keys[0]
      when Symbol,String then args.to_s
    end
    Gnuplot.open do |gp|
      Gnuplot::Plot.new(gp) do |plot|
        $current_plot = plot;
        plot.terminal $plot_terminal
        if $plot_suffix and task_name then
          plot.output "#{task_name}#{$plot_suffix}"
        end
        yield plot;
      end
    end  
    if $plot_auto_open and [".pdf", ".png"].include? $plot_suffix
      system("open #{task_name}#{$plot_suffix}")
    end
  end
end

def line_plot(args = {})
  plot(args) do |plot|
    data_elements = yield(plot)
    data_elements = { :data => data_elements } unless data_elements.is_a? Hash;
    
    data = data_elements[:data].unzip;
    xaxis = data_elements.fetch(:xaxis) { data.shift };
    keys = data_elements.fetch(:keys) { data.map { nil; } }
    withs = data_elements.fetch(:with, "linespoints");
      withs = data.map { withs } unless withs.is_a? Array;
    
    raise "Missing data!" if data.nil?;
    raise "Missing X Axis!" if xaxis.nil?;
    
    data.zip(keys, withs).each do |line, key, with|
      plot.data << Gnuplot::DataSet.new([xaxis, line]) do |ds|
        ds.title = key unless key.nil?
        ds.with = with unless with.nil?
      end
    end
  end
end

def draw_clustered_bar_plot plot, args = {}
  data = args.fetch(:data).unzip;
  base_offset       = args.fetch(:base_offset,       0);
  interbar_offset   = args.fetch(:interbar_offset,   18);
  intergroup_offset = args.fetch(:intergroup_offset, interbar_offset);
  margins           = args.fetch(:margins,           intergroup_offset);
  bar_width         = args.fetch(:bar_width,         10);
  tic_commands      = args.fetch(:tic_commands,      "");
  label_offset      = args.fetch(:label_offset,      0);
  box_style         = args.fetch(:box_style, 
                                 lambda { |i| "boxes fill pattern #{i}" });
  
  group_offset = base_offset + margins
  group_size = interbar_offset * data.length + intergroup_offset;
  plot.boxwidth bar_width.to_s;
  pattern = 0;
  data.zip(args[:dataset_labels]).each do |dataset, dataset_title|
    offset = group_offset - group_size;
    group_offset += interbar_offset;
    
    indices = dataset.map { |i| offset += group_size; }
    plot.data << Gnuplot::DataSet.new([indices,dataset]) do |ds|
      ds.title = dataset_title
      ds.with  = box_style.call(pattern += 1);
    end
  end
  
  label_offset += (group_size+intergroup_offset-margins)/2
  group_offset = base_offset - label_offset;
  plot.xtics "(#{args[:group_labels].map do |label|
    "\"#{label}\" #{group_offset += group_size}";
  end.join(", ")}) scale 0 #{tic_commands}";
  
  plot.xrange "[-10:#{group_offset+label_offset+margins-intergroup_offset}]"
end

def draw_bar_plot plot, args
  plot.key "off"
  args = args.clone
  args[:data] = args[:data].map {|d| [d]}
  args[:dataset_labels] = [""];
  args[:group_labels] = args[:labels];
  
  draw_clustered_bar_plot plot, args
end

