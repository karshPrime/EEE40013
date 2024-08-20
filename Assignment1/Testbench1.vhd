
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library std;
use std.textio.all;

entity testbench is
end testbench;

architecture behavior of testbench is

    -- ports of UUT
    signal Clock    : STD_LOGIC := '0';
    signal A        : unsigned(15 downto 0) := (others => '0');
    signal B        : unsigned(15 downto 0) := (others => '0');
    signal Q        : unsigned(31 downto 0) := (others => '0');
    signal Start    : STD_LOGIC := '0';
    signal Complete : STD_LOGIC := '0';
    signal Reset    : STD_LOGIC := '0';

    -- File to log results to
    file logFile : TEXT;

    constant clockHigh : TIME := 50 ns;
    constant clockLow : TIME := 50 ns;
    constant clockPeriod : TIME := clockHigh + clockLow;

    signal simComplete : BOOLEAN := false;

begin

    --**********************************************************************************************
    -- Clock Generator
    ClockGen : process
    begin
        while not simComplete loop
            Clock <= '0';
            wait for clockHigh;
            Clock <= '1';
            wait for clockLow;
        end loop;

        wait; -- stop process looping
    end process ClockGen;

    --*****************************************************************************
    -- Stimulus Generator
    Stimulus : process

        --*************************************************************************
        -- Write a message to the logfile & transcript
        -- message => string to write
        procedure writeMsg (message : STRING) is
            variable assertMsgBuffer : STRING(1 to 4096); -- string for assert message
            variable writeMsgBuffer : line; -- buffer for write messages
        begin
            write(writeMsgBuffer, message);
            assertMsgBuffer(writeMsgBuffer.all'range) := writeMsgBuffer.all;
            writeline(logFile, writeMsgBuffer);

            deallocate(writeMsgBuffer);
            report assertMsgBuffer severity note;
        end;

        --*************************************************************************
        -- Perform multiplication and log the results
        --  stimulusA, stimulusB => values to multiply
        procedure doMultiply(stimulusA : unsigned(15 downto 0);
                             stimulusB : unsigned(15 downto 0))
        is
            variable assertMsgBuffer : STRING(1 to 4096); -- string for assert message
            variable writeMsgBuffer : line; -- buffer for write messages
        begin
            A <= stimulusA;
            B <= stimulusB;

            Start <= '1';
            wait until rising_edge(Clock);
            Start <= '0';

            wait until Complete = '1'; -- Wait for the complete signal to go high

            write(writeMsgBuffer, STRING'("A = "), left);
            write(writeMsgBuffer, to_integer(stimulusA));

            write(writeMsgBuffer, STRING'(", | B = "), left);
            write(writeMsgBuffer, to_integer(stimulusB));

            write(writeMsgBuffer, STRING'(" | ExpectedQ = "), left);
            write(writeMsgBuffer, to_integer(stimulusA * stimulusB));

            write(writeMsgBuffer, STRING'(" | GeneratedQ = "), left);
            write(writeMsgBuffer, to_integer(Q));

            assertMsgBuffer(writeMsgBuffer.all'range) := writeMsgBuffer.all;
            writeline(logFile, writeMsgBuffer);
            deallocate(writeMsgBuffer);
            report assertMsgBuffer severity note;
        end;

        variable openStatus : file_open_status;

    begin -- Stimulus

        file_open(openStatus, logFile, "results.txt", WRITE_MODE);

        writeMsg(STRING'("Simulation starting."));

        -- Initial reset
        A <= (others => '0');
        B <= (others => '0');
        Start <= '0';
        Reset <= '1';
        wait for 10 ns;
        Reset <= '0';
        wait until falling_edge(Clock);

        -- Test cases
        doMultiply("0000000000111010", "0000000000111111"); -- 58 x 63
        doMultiply("0000000000001111", "0000111111111111"); -- 15 x 1023
        doMultiply("0000000000000000", "1111111111111111"); -- 0  x MAX
        doMultiply("0000000000000010", "0101010101010101"); -- 2  x something; shift 1 left
        doMultiply("0000000000010000", "0000111000110010"); -- 16 x something; shift 4 left
        doMultiply("0000000000000100", "0011011111101100"); -- 4  x something; shift 2 left

        wait for 20 ns;

        writeMsg(STRING'("Simulation completed."));
        file_close(logFile);

        simComplete <= true;
        wait;

    end process Stimulus;

    uut : entity work.Multiplier1Cycle
    port map
    (
        Reset    => Reset,
        Clock    => Clock,
        A        => A,
        B        => B,
        Q        => Q,
        Start    => Start,
        Complete => Complete
    );
end;
