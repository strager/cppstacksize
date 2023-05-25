void f() {
    int before_blocks = 0;
    {
        int before_innermost_blocks = 1;
        {
            int inside_first_innermost_block = 2;
        }
        int between_innermost_blocks = 3;
        {
            int inside_second_innermost_block = 4;
        }
        int after_innermost_blocks = 5;
    }
    int after_blocks = 6;
}
