//
// Created by daniil on 20.12.2021.
//

#ifndef OSCOURSE_NIST_H
#define OSCOURSE_NIST_H

#include <inc/crng.h>
#include <inc/math.h>
#include <stdint.h>

#define bool _Bool
#define true 1
#define false 0

///First and basic test. If this test fails, the likelihood of other tests failing is high.
bool frequency_test(unsigned n, unsigned not_used, uint64_t (*func)());

///The focus of the test is the proportion of ones within M-bit blocks.
bool frequency_block_test(unsigned n, unsigned M, uint64_t (*func)());

///The focus of this test is the total number of runs in the sequence, where a run is an uninterrupted sequence of identical bits.
bool runs_test(unsigned n, unsigned not_used, uint64_t (*func)());

///The focus of the test is the longest run of ones within M-bit blocks.
///Params: n=128, M=8; n=6272, M=128
//TODO: add n=750.000, M=10.000
bool longest_run_of_ones_test(unsigned n, unsigned M, uint64_t (*func)());

#endif // OSCOURSE_NIST_H
