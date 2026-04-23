## Requirements

### Requirement: GitHub Pages SHALL explain HTS as a product

The GitHub Pages site SHALL present HTS through a product-focused landing experience that explains
what the scheduler does, where it fits, how the architecture is organized, and how users can
evaluate or adopt it, and the site SHALL not be a line-by-line mirror of the repository README.

#### Scenario: New visitor lands on the homepage

- **WHEN** the visitor opens the GitHub Pages home page
- **THEN** they can understand HTS value, architecture, and next steps without reading the README
  first

### Requirement: Public-facing claims SHALL be evidence-based

README, website, and repository metadata SHALL avoid unverifiable testimonials or performance claims
unless they are sourced, qualified, or demonstrably supported by the repository.

#### Scenario: Maintainer reviews public marketing copy

- **WHEN** a claim cannot be supported by repository evidence or clearly qualified context
- **THEN** the claim is removed, softened, or replaced with factual project description

### Requirement: GitHub repository metadata SHALL align with the maintained landing surface

The repository About description, homepage URL, and curated topics SHALL describe HTS consistently
with the maintained GitHub Pages site and current project scope.

#### Scenario: Repository metadata is updated during closeout

- **WHEN** a maintainer inspects the repository About section after normalization
- **THEN** the description, homepage, and topics point to the maintained project story rather than
  stale or incomplete metadata

### Requirement: Documentation navigation SHALL prioritize onboarding value

The maintained public documentation surface SHALL prioritize getting started guidance, architecture
explanation, examples, API orientation, and contribution entry points over internal process history.

#### Scenario: Visitor navigates the docs site

- **WHEN** the visitor opens the top-level navigation
- **THEN** the primary paths guide them toward product understanding and practical evaluation rather
  than low-signal internal bookkeeping pages
