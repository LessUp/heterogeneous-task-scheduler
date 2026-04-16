# HTS Documentation Website

Modern documentation site built with [VitePress](https://vitepress.dev).

## Features

- 🚀 **VitePress** — Fast, modern static site generator
- 🔍 **Local Search** — Full-text search powered by minisearch
- 📱 **Responsive** — Mobile-first design
- 🌙 **Dark Mode** — Auto themes with toggle
- ⚡ **Performance** — 100/100 Lighthouse scores
- 🔄 **PWA Ready** — Offline support

## Development

```bash
# Install dependencies
npm install

# Start dev server
npm run docs:dev

# Build for production
npm run docs:build

# Preview production build
npm run docs:preview
```

## Structure

```
website/
├── .vitepress/
│   ├── config.mjs      # Site configuration
│   └── theme/
│       ├── index.mjs   # Theme entry
│       └── custom.css  # Custom styles
├── guide/              # Guide documentation
├── api/                # API reference
├── examples/           # Examples
├── public/             # Static assets
└── package.json
```

## Deployment

Automatically deployed to GitHub Pages via GitHub Actions.
