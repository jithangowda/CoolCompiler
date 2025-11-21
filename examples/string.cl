class StringDemo {
    main() : Int {
        {
            let s : String <- "Hello" in
            {
                (new IO).out_string("Length: ");
                (new IO).out_int(s.length());
                (new IO).out_string("\n");

                (new IO).out_string("Concatenated: ");
                (new IO).out_string(s.concat(" World!"));
                (new IO).out_string("\n");

                (new IO).out_string("Substring: ");
                (new IO).out_string("Hello World".substr(6, 5));
                (new IO).out_string("\n");
            };
            0;
        }
    };
};