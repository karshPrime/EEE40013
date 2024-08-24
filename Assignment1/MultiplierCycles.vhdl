library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity MultiplierCycles is
    port
    (
        Clock    : in  STD_LOGIC;
        Reset    : in  STD_LOGIC;
        A        : in  unsigned(15 downto 0);
        B        : in  unsigned(15 downto 0);
        Q        : out unsigned(31 downto 0);
        Start    : in  STD_LOGIC;
        Complete : out STD_LOGIC);
end MultiplierCycles;


architecture Behavioural of MultiplierCycles is

    type TheStates is (IDLE, OP1, OP2, OP3, DONE);
    signal state : TheStates;

    -- holds 8-bit segment of input A and B for partial multiplication
    signal A_Byte, B_Byte : unsigned(7 downto 0) := (others => '0');

    -- signal to control what to add with the 17bit adder
    signal Add17  : unsigned(16 downto 0) := (others => '0');

    -- accumulator for the final result
    signal Acm    : unsigned(31 downto 0) := (others => '0');

    -- aliases for different accumulator segments
    alias Acm_MID : unsigned(16 downto 0) is Acm(24 downto 8);  -- middle
    alias Acm_END : unsigned(15 downto 0) is Acm(31 downto 16); -- end

begin

    -- Control state machine
    StateMachine : process (Reset, Clock)
    begin
        if (Reset = '1') then
            state    <= IDLE;
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
                when OP3 => state <= DONE;

                when DONE =>
                    state <= IDLE;
                    Complete <= '1';
            end case;
        end if;
    end process StateMachine;

    -- Combinational logic to compute partial products
    Comb : process (state, A, B, Acm)
    begin
        Add17 <= Acm_MID; -- default; middle 17bits of the accumulator
        case state is
            -- when IDLE =>
            --     A_Byte <= (others => '0');
            --     B_Byte <= (others => '0');

            when IDLE | OP1  =>
                A_Byte <= A(7 downto 0);
                B_Byte <= B(7 downto 0);

            when OP2  =>
                A_Byte <= A(7  downto 0);
                B_Byte <= B(15 downto 8);

            when OP3  =>
                A_Byte <= A(15 downto 8);
                B_Byte <= B(7  downto 0);

            when DONE =>
                A_Byte <= A(15 downto 8);
                B_Byte <= B(15 downto 8);
                Add17  <= '0' & Acm_END; -- most significant bits of accumulator
        end case;
    end process Comb;

    -- Synchronous logic for the accumulator and output
    Synch : process (Reset, Clock)
		variable Zero16      : unsigned(15 downto 0);
        variable AB_multiple : unsigned(15 downto 0);
        variable Sum17       : unsigned(16 downto 0);
    begin
        if (Reset = '1') then
            Acm <= (others => '0');

        elsif rising_edge(Clock) then
            AB_multiple := A_Byte * B_Byte;
			Zero16    := ( others => '0' );

            case state is
                -- when IDLE => Acm <= (others => '0');

                when IDLE | OP1 =>
                    Acm <= Zero16 & AB_multiple;

                when OP2  | OP3 =>
                    Acm_MID <= Add17 + AB_multiple;

                when DONE =>
                    Sum17   := Add17 + AB_multiple;
                    Acm_END <= Sum17(15 downto 0);
            end case;
        end if;
    end process Synch;

    Q <= Acm;

end Behavioural;
