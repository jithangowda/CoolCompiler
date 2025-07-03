class Main {
    main(): Object {
        let x: Int <- 5 in {
            if x < 10 then
                out_string("Less than 10\n")
            else
                out_string("10 or more\n")
            fi;
        }
    };
};
