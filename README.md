# Quick Subset Construction
*An algorithm for quick determinization of NFAs to DFAs*

Quick Subset Construction (**QSC**) is an algorithm developed by Gianfranco Lamperti and Michele Dusi for the determinization of Non-deterministic Finite Automata (**NFA**) and Epsilon Non-deterministic Finite Automata (**ε-NFA**).

It's proven to compute the Deterministic Finite Automaton (**DFA**) equivalent to the result of Subset Construction, the standard determinization algorithm in literature. While the latter builds the result from scratch, QSC works on the original automaton, operating only on the spots that need an intervention. Thus, we claim QSC to have a *conservative* approach to the problem.
For this, there are cases where QSC performs significantly better than SC.

This repository contains a simple implementation of QSC and SC within a testing framework that can measure the performance of the two algorithms. The two approaches are compared in the determinization of random automata, generated according to some parameters. The framework offers a resume of the performances, allowing to choose the best algorithm for the testcases.

For more information, please see the paper "Quick Subset Construction" *[page TBA]*.
