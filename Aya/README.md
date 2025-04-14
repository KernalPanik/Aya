# Aya

Metamorphic Relation Search Tool

## What is Metamorphic Relation?

*Metamorphic Relation* is a relation between sequences (chains) of transformations performed on a given function (program) inputs, and matching transformations on the final result, or a part of the result.

*Consider function Sin(x).*

From trigonometry, we know that
```
Sin^2(x) = 1 - Cos^2(x)
```

This rule is valid for any x, no matter how transformed it is. Thus, this can be seen as a Metamorphic Relation between input *x* and the result of function *Sin^2(x)*.

### Benefits for Software Engineering

Valid Metamorphic Relations are useful as small specifications for our software, and are very friendly for later conversions into tests -- either unit if function being tested is a unit in itself, or even integration if function we're testing encompasses multiple components. Most importantly, such tests don't require a pre-calculated oracle, as they are oracles in themselves. For example, standard unit tests for *Sin^2(x)* would require us to get trusted implementation of *Sin^2(x)*, get outputs from it, and use them as oracles. With Metamorphic Relation testing, we'd simply verify if rule above is valid, alongside with very simple tests to check if Sine function works, e.g. checking if *Sin(90deg) == 1*.

## Why Aya?

Aya allows to operate on abstract functions of any signature, and transformers of various argument lists. It allows to transform and compare user-defined elements of produced states, allowing to cover data structures or algorithms outside of strictly mathematical libraries.

## Aya MR Generation Procedure

## Tested Functions

## How To Build & Run