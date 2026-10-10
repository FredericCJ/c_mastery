# Search, reflection, and design decisions for C Mastery 4

Baseline: v3 commit `4f3e3a2`. The work starts from the existing source catalog and
local teaching documents, rather than replacing the curriculum's technical scope.

## What the audit found

- 56 domains, 627 micro-topics, 1,881 objectives, and 429 projects remain required or
  elective according to their existing dispositions. Project implementation and
  verification sum to 3,599 hours within the 4,879-hour full catalog.
- Nine executable families contain 261 memberships and 259 unique projects.
  Only 235 unique projects have a declared test selection. Shared anchor tests do
  not implement every project that cites them. There are 170 unmapped projects
  and 24 with recipes only.
- The legacy project ordering sorts largely by domain ID. Basic OS observation
  appears at step 338; basic boot waits for final VM and storage projects even
  though those components are consumed only in later kernel integrations.
- All 429 legacy graph outputs use the generic accepted-component-and-dossier
  suffix. This loses the identity and assumptions of the actual reusable code.
- Delayed retrieval and a 0–3 rubric already exist. The old CLI does not schedule
  retrieval, preserve an attempt ledger, or seal a design before implementation.
- Some generated working-card questions discuss mechanisms absent from their
  projects. V4 replaces those cards in its publication overlay while preserving
  the historical source bytes and migration contracts.

## Source search and limits

Technical reading continues to use the inspected 204-record reference library,
whose local locators point into `F:\Knowledge`. The SWE and data-structure graphs
are discovery indexes, not substitutes for the underlying documents. Each new
laboratory binds its mechanism to existing technical source passages; the v4
topic cards inherit bounded module readings and identify the relevant objectives.

A filename-oriented search of the local library did not find an obvious matching
learning-science collection. This is not evidence that no such works exist.
Primary publications were inspected online for the new retrieval design:

1. Karpicke and Blunt (2011), *Retrieval Practice Produces More Learning than
   Elaborative Studying with Concept Mapping*, Science 331:772–775,
   DOI 10.1126/science.1199327. Author-hosted paper:
   https://learninglab.psych.purdue.edu/downloads/2011/2011_Karpicke_Blunt_Science.pdf
2. Karpicke and Bauernschmidt (2011), *Spaced Retrieval: Absolute Spacing Enhances
   Learning Regardless of Relative Spacing*, JEP:LMC 37:1250–1257,
   DOI 10.1037/a0023436. Author-hosted paper:
   https://learninglab.psych.purdue.edu/downloads/2011/2011_Karpicke_Bauernschmidt_JEPLMC.pdf
3. YeckehZaare and Resnick (2025), *Counting Days Is a Spacing Incentive That
   Unlocks the Potential of Low GPA Students*, npj Science of Learning 10:35,
   DOI 10.1038/s41539-025-00322-5:
   https://www.nature.com/articles/s41539-025-00322-5

The first two studies support retrieval and spacing but do not establish an
optimal schedule for advanced C. The programming study concerns spacing
incentives; it does not justify promising a fixed improvement from this workflow.
The 1/3/7/14/30-day gaps and 14-day handoff freshness rule are adjustable design
defaults. We will calibrate workload with learner reports, not treat author tests
as evidence of learning effectiveness.

## Chosen response

Keep the existing budget and identity catalog. Add short units, concrete artifact
lineages, meaningful design gates, and a bounded recall queue. Practice remains
accessible while acceptance requires the relevant evidence. Build the CLI and
paper route from the same records. Preserve all required physical and strict
freestanding obligations; host tests cannot discharge them.
