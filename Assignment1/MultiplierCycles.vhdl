
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity Multiplier1Cycle is
    port
    (
        Clock    : in  STD_LOGIC;
        Reset    : in  STD_LOGIC;
        A        : in  unsigned(15 downto 0);
        B        : in  unsigned(15 downto 0);
        Q        : out unsigned(31 downto 0);
        Start    : in  STD_LOGIC;
        Complete : out STD_LOGIC);
end Multiplier1Cycle;
architecture Behavioral of Multiplier1Cycle is

    type TheStates is (IDLE, OP1, OP2, OP3, OP4, DONE);
    signal state : TheStates;

    -- holds the 16-bit result of an 8-bit input partial multiplication
    signal Input_Mul : unsigned(15 downto 0) := (others => '0');

    -- accumulator for the final result
    signal Acm     : unsigned(31 downto 0) := (others => '0');
    alias  Acm_SRT : unsigned(15 downto 0) is Acm(15 downto 0);  -- start segment of accumulator
    alias  Acm_MID : unsigned(16 downto 0) is Acm(24 downto 8);  -- middle segment of accumulator
    alias  Acm_END : unsigned(15 downto 0) is Acm(31 downto 16); -- end segment of accumulator

begin

    -- Control state machine
    StateMachine : process (Reset, Clock)
    begin
        if (Reset = '1') then
            state <= IDLE;
            Complete <= '0';

        elsif rising_edge(Clock) then
            case state is
                when IDLE =>
                    Complete <= '0';
                    if (Start = '1') then
                        state <= OP1;
                    end if;

                when OP1 => state <= OP2;
                when OP2 => state <= OP3;
                when OP3 => state <= OP4;
                when OP4 => state <= DONE;

                when DONE =>
                    state <= IDLE;
                    Complete <= '1';
            end case;
        end if;
    end process StateMachine;

    -- Combinational logic to compute partial products
    Comb : process (state, A, B)
    begin
        case state is
            when OP1 => Input_Mul <= A(7 downto 0) * B(7 downto 0);
            when OP2 => Input_Mul <= A(7 downto 0) * B(15 downto 8);
            when OP3 => Input_Mul <= A(15 downto 8) * B(7 downto 0);
            when OP4 => Input_Mul <= A(15 downto 8) * B(15 downto 8);
            when others => Input_Mul <= (others => '0'); -- IDLE or DONE
        end case;
    end process Comb;

    -- Synchronous logic for the accumulator and output
    Synch : process (Reset, Clock)
    begin
        if (Reset = '1') then
            Q <= (others => '0');
            Acm <= (others => '0');
        elsif rising_edge(Clock) then
            case state is
                when IDLE      => Acm <= (others => '0');
                when OP1       => Acm_SRT <= Input_Mul;
                when OP2 | OP3 => Acm_MID <= Acm_MID + Input_Mul;
                when OP4       => Acm_END <= Acm_END + Input_Mul;
                when DONE      => Q <= Acm;
            end case;
        end if;
    end process Synch;

end Behavioral;
