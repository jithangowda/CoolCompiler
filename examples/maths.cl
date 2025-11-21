class MathOps {
    a : Int <- 10;
    b : Int <- 5;

    main() : Int {
        {
            (new IO).out_int(a + b);    -- 15
            (new IO).out_string("\n");
            (new IO).out_int(a - b);    -- 5
            (new IO).out_string("\n");
            (new IO).out_int(a * b);    -- 50
            (new IO).out_string("\n");
            (new IO).out_int(a / b);    -- 2
            (new IO).out_string("\n");
            0;
        }
    };
};