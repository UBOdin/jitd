class JitdController < ApplicationController

  def show
    respond_to do |format|
      format.json do
        render json:{ 
          "name" => "BTree(23)",
          "children" => [
            { "name" => "Array(2000 elements)" },
            { "name" => "Array(2000 elements)" }
          ]
        }
      end
    end
  end

end
