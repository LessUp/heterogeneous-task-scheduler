# Documentation

The maintained public documentation surface for HTS is the GitHub Pages site:

- <https://lessup.github.io/heterogeneous-task-scheduler/>

## Source documents (Migrated to website/)

**All documentation has been consolidated into `website/` for VitePress publishing:**

- `website/guide/` - English guides
- `website/api/` - English API reference
- `website/examples/` - English examples
- `website/zh-CN/guide/` - Chinese guides
- `website/zh-CN/api/` - Chinese API reference
- `website/zh-CN/examples/` - Chinese examples

## Building Documentation

```bash
cd website
npm install
npm run docs:build
```

## Specifications

- Durable specs: [`../specs/README.md`](../specs/README.md)
- Active change artifacts: `../openspec/changes/<change-name>/`

## Related entry points

- [`../README.md`](../README.md) - Project overview and quick start
- [`../CONTRIBUTING.md`](../CONTRIBUTING.md) - Contribution workflow
- [`../AGENTS.md`](../AGENTS.md) - Project operating rules
