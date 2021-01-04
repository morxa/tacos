# Pseudo Code (Timed Games)

1. Construct $\mathit{DTA}(\Sigma, \delta)$ for Golog program $\delta$ with BAT $\Sigma$ for some platform model $\mathcal{R}$
1. Construct an ATA $B_\varphi$ with $\mathcal{L}^*(\mathcal{B}_\varphi) = \mathcal{L}^*(\varphi)$, following [^OW2005]

   TODO: More details on the construction of $B_\varphi$

1. ~~Construct $\mathcal{T}_{\mathcal{A}/\varphi}$, the synchronous product of $\mathcal{A}$ and $\mathcal{B}_\varphi$~~

   _Do not construct the complete synchronous product, it has uncountably many states_
1. Construct $DT_\sim$ on the fly as follows:
   * Start with $w_0 = H(s_0)$, where $H(s)$ is a canonical word representing $s$; $H(\cdot)$ is an equivalence relation that partitions $\mathcal{T}_{\mathcal{A}/\varphi}$

     Canonical word $H$:
     * Each $s$ is an $\mathcal{A}/\mathcal{B}_\varphi$ configuration $s = ((q, \nu), G)$, where
       * $q$ is the state of $\mathcal{A}$,
       * $\nu$ is a clock valuation over the set of clocks of $\mathcal{A}$
       * $G$ is a configuration of $\mathcal{B}_\varphi$, i.e., a set of ATA states $\{ (s_i, \nu_i) \}$, where $s_i$ is a location in $\mathcal{B}\varphi$ and $\nu_i$ is a clock valuation for the single clock of $\mathcal{B}_\varphi$
     * $H(s) \in \Lambda = 2^{(Q \times X \times \mathit{REG}_K) \cup (P \times
       \mathit{REG}_K)}$, where each letter describes the state of either
       $\mathcal{A}$ or $\mathcal{B}_\varphi$.
       More specifically, each letter is either
       * a pair $(p, r)$,  where p is a location of $\mathcal{B}_\varphi$ and $r$ is the regionalization of $\mathcal{B}_\varphi$'s (unique) clock
       * a triple $(q, y, r)$, where $q$ is a location of $\mathcal{A}$, $y$ is a clock of $\mathcal{A}$, and $r$ is the regionalization of the clock valuation of $y$

     * In a word $w \in H(s)$, the letters are sorted by their fractional part.
     * $T_\sim$ is the regionalized synchronous product of $\mathcal{A}$ and $\mathcal{B}_\varphi$
     * $\mathit{reg}_\mathcal{A}(w)$ (where $w \in W$) is the projection of $w$ onto the regionalized state of $\mathcal{A}$
     * To obtain $\mathit{DT}_\sim$, we use $\mathit{SW} = \{ \mathcal{C}_i \}_i$, which are equivalence classes of those projected words
     * It may happen that $\mathit{reg}_\mathcal{A}(w_1) = \mathit{reg}_\mathcal{A}(w_2)$, but we are in different $\mathcal{B}_\varphi$ states, therefore $\mathcal{C}_i$ is a set
     * $\mathit{DT}_\sim$ is the restriction of $\mathit{Det}(T_\sim)$ to states from $\mathit{SW}$; the transition relation is taken from $\mathit{Det}(T_\sim)$
   * A transition $\mathcal{C} \overset{a, g, Y}{\longrightarrow} \mathcal{C}'$
     is added on-demand if there are $s_1 \in H^{-1} (w_1), s_2 \in H^{-1} (w_2)$ with $s_1 \overset{a,g,Y}{\longrightarrow} s_2$

     Question: How do we compute $H^{-1}$?

1. Perform breadth-first search on the unfolding of $DT_\sim$ (unfold on demand) and label states as follows:
    * label fixed points as successful
    * label bad states as unsuccessful
    * label states with no successors as dead
    * otherwise: compute successors in the tree
1. Afterwards: label bottom-up: successful and dead nodes are marked with TRUE, unsuccessful nodes with FALSE. For internal nodes: if all successors are labelled with TRUE, label with TRUE, otherwise with FALSE (Todo: revise this, I am not 100% sure aboute the ALL successors).
1. If the root is labeled with TRUE, there exists a winning strategy.

## $H(s)$

Given a $\mathcal{A}/\mathcal{B}_\varphi$ configuration $s = ((q, \nu), G)$,
where $G$ is a $\mathcal{B}_\varphi$ configuration, i.e., a set of ATA states
$\{ (s_i, \nu_i) \}$:

1. Explode $(q, \nu)$ to a set $\{(q, y_i, \nu(y_i))\}_{y_i \in X}$
2. Compute $G' = G \cup \{(q, y_i, \nu(y_i))\}_{y_i \in X}$
3. Sort $G'$ by fractional part of the clock valuations
4. Regionalization: Replace each member $G_i$ of $G'$ by $Abs(G_i) = \{ (p, \mathit{reg}(u)) | (p, u) \in G_i) \} \cup \{ (q, y, \mathit{reg}(u)) | (q, y, u) \in G_i \}$, by replacing each clock valuation by the region index of the clock

[^OW2005]: J. Ouaknine and J. Worrell, “On the decidability of metric temporal logic,” in 20th Annual IEEE Symposium on Logic in Computer Science (LICS’ 05), 2005, pp. 188–197, doi: 10.1109/LICS.2005.33.
