#pragma once

// Running a test and expecting and assert within it. If no assert exception is thrown, considering test as [OK]
void RunTest(void*(test)());