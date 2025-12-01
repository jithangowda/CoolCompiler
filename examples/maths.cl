class Main {
  a : Int <- 10;
  b : Int <- 5;

  main() : Int {
    {
      (new IO).out_int(a + b);    -- Should print 15
      
      (new IO).out_int(a - b);    -- Should print 5
      
      (new IO).out_int(a * b);    -- Should print 50
      
      (new IO).out_int(a / b);    -- Should print 2
      
      0;
    }
  };
};