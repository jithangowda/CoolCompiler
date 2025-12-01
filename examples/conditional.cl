class Main {
  x : Int <- 10;
  y : Int <- 20;

  main() : Int {
    {
      (new IO).out_string("Testing if-else:\n");
      
      if x < y then
        (new IO).out_string("x is less than y\n")
      else
        (new IO).out_string("x is not less than y\n")
      fi;

      if x = 10 then
        (new IO).out_string("x equals 10\n")
      else
        (new IO).out_string("x does not equal 10\n")
      fi;

      0;
    }
  };
};