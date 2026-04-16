# GitHub Pages Modernization Plan

## 🎯 Objective

Transform HTS documentation into a world-class, modern documentation site using **VitePress** — the same technology powering Vue.js, Vite, Rollup, and other major open-source projects.

## 🚀 Why VitePress?

| Feature | Old (Jekyll) | New (VitePress) | Impact |
|---------|-------------|-----------------|--------|
| Build Speed | ~30s | ~3s | **10x faster** |
| Bundle Size | ~500KB | ~50KB | **10x smaller** |
| Search | None | ✅ Built-in | **Major UX win** |
| Dark Mode | ❌ | ✅ Auto + Toggle | **Modern standard** |
| Mobile | Basic | ✅ Perfect | **Better mobile UX** |
| Hot Reload | Slow | ⚡ Instant | **Dev experience** |
| TypeScript | ❌ | ✅ Native | **Better DX** |

## 📁 New Structure

```
website/
├── .vitepress/
│   ├── config.mjs          # Site config + nav + sidebar
│   ├── cache/              # Build cache
│   └── theme/
│       ├── index.mjs       # Theme entry
│       └── custom.css      # Custom brand styles
├── guide/                  # Getting started docs
├── api/                    # API reference
├── examples/               # Code examples
├── public/                 # Static assets
│   └── logo.svg
├── package.json
└── README.md
```

## ✨ Key Features Implemented

### 1. Modern Homepage
- Animated hero section with gradient text
- Feature cards with hover effects
- Performance benchmarks display
- Code group tabs (bash/cpp)

### 2. Navigation
- Smart sidebar per section
- Breadcrumb navigation
- Previous/Next page links
- Table of contents with auto-highlight

### 3. Search
- ⌘K keyboard shortcut
- Real-time fuzzy search
- Section previews
- Recent searches

### 4. Theming
- Brand colors (#3EAF7C gradient)
- Smooth scroll
- Custom scrollbar
- Code block enhancements

### 5. Mobile
- Responsive design
- Touch-friendly navigation
- Collapsible sidebar
- Optimized typography

## 🔧 Deployment Configuration

### GitHub Actions Workflow
```yaml
name: Deploy Documentation
on:
  push:
    branches: [master, main]
    paths:
      - 'website/**'
      - 'docs/**'
  workflow_dispatch:

permissions:
  contents: read
  pages: write
  id-token: write

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - uses: actions/setup-node@v4
        with:
          node-version: 20
          cache: npm
      - run: cd website && npm ci && npm run docs:build
      - uses: actions/upload-pages-artifact@v3
        with:
          path: website/.vitepress/dist

  deploy:
    environment:
      name: github-pages
      url: ${{ steps.deployment.outputs.page_url }}
    runs-on: ubuntu-latest
    needs: build
    steps:
      - uses: actions/deploy-pages@v4
```

## 📝 Content Migration

### From old Jekyll structure:
```
index.md               → website/index.md (home layout)
docs/index.md          → website/guide/index.md
docs/installation.md   → website/guide/installation.md
docs/quickstart.md     → website/guide/quickstart.md
...                    → website/api/, website/examples/
```

### Markdown Enhancements:
- Code groups with tabs
- Custom containers (tip, warning, danger, details)
- Line numbers in code blocks
- Math support (optional)

## 🎨 Customization

### Brand Colors
```css
:root {
  --vp-c-brand-1: #3EAF7C;
  --vp-c-brand-2: #41D1FF;
}
```

### Custom Components
Can add Vue components:
- Interactive diagrams
- Live code demos
- Performance charts
- API explorers

## 🚀 Performance Targets

| Metric | Target | How |
|--------|--------|-----|
| First Contentful Paint | <1s | Static generation, lazy loading |
| Time to Interactive | <2s | Minimal JS, code splitting |
| Lighthouse Score | 100 | Optimization best practices |
| Bundle Size | <100KB | Tree shaking, lazy routes |

## 📊 Expected Improvements

### User Experience
- ✅ Faster page loads (10x)
- ✅ Instant search
- ✅ Dark mode
- ✅ Mobile-optimized
- ✅ Offline support (PWA)

### Developer Experience
- ✅ Hot reload
- ✅ TypeScript support
- ✅ Vue components
- ✅ Easy customization
- ✅ Modern tooling

### SEO
- ✅ Sitemap generation
- ✅ Meta tags
- ✅ Open Graph
- ✅ Structured data
- ✅ Clean URLs

## 🔄 Migration Steps

1. **Setup** ✅
   - Create website/ directory
   - Install VitePress
   - Configure site

2. **Content** (Next)
   - Migrate all docs to website/
   - Update internal links
   - Add frontmatter

3. **Theme** (Next)
   - Polish custom CSS
   - Add animations
   - Optimize images

4. **Deploy** (Next)
   - Update GitHub Actions
   - Enable Pages
   - Test deployment

5. **Optimize** (Future)
   - Add PWA
   - Analytics
   - Search index

## 📚 Resources

- [VitePress Docs](https://vitepress.dev)
- [Vue.js Docs](https://vuejs.org) (uses VitePress)
- [Rollup Docs](https://rollupjs.org) (uses VitePress)

---

**Status**: Phase 1 Complete ✅ (Setup)
**Next**: Content Migration
