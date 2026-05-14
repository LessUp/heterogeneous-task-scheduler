import { defineConfig } from 'vitepress'
import { withMermaid } from 'vitepress-plugin-mermaid'
import llmstxt from 'vitepress-plugin-llms'

// Dynamic base path for GitHub Pages
const rawBase = process.env.VITEPRESS_BASE
const base = rawBase
  ? rawBase.startsWith('/')
    ? rawBase.endsWith('/') ? rawBase : `${rawBase}/`
    : `/${rawBase}/`
  : '/heterogeneous-task-scheduler/'

// Sidebar functions for English
function sidebarGuideEn() {
  return [
    {
      text: 'Getting Started',
      collapsed: false,
      items: [
        { text: 'Introduction', link: '/en/guide/' },
        { text: 'Installation', link: '/en/guide/installation' },
        { text: 'Quick Start', link: '/en/guide/quickstart' },
        { text: 'Architecture', link: '/en/guide/architecture' }
      ]
    },
    {
      text: 'Core Topics',
      collapsed: false,
      items: [
        { text: 'Task Graph', link: '/en/guide/task-graph' },
        { text: 'Scheduling', link: '/en/guide/scheduling' },
        { text: 'Memory', link: '/en/guide/memory' },
        { text: 'Error Handling', link: '/en/guide/error-handling' }
      ]
    }
  ]
}

function sidebarAPIEn() {
  return [
    {
      text: 'Core API',
      items: [
        { text: 'Overview', link: '/en/api/' },
        { text: 'Scheduler', link: '/en/api/scheduler' },
        { text: 'TaskGraph', link: '/en/api/task-graph' },
        { text: 'TaskBuilder', link: '/en/api/task-builder' }
      ]
    }
  ]
}

function sidebarExamplesEn() {
  return [
    {
      text: 'Examples',
      items: [
        { text: 'Overview', link: '/en/examples/' },
        { text: 'Simple DAG', link: '/en/examples/simple-dag' },
        { text: 'Pipeline', link: '/en/examples/pipeline' }
      ]
    }
  ]
}

function sidebarDesignEn() {
  return [
    {
      text: 'Design',
      items: [
        { text: 'Philosophy', link: '/en/design/philosophy' }
      ]
    }
  ]
}

function sidebarBenchmarksEn() {
  return [
    {
      text: 'Benchmarks',
      items: [
        { text: 'Overview', link: '/en/benchmarks/' }
      ]
    }
  ]
}

function sidebarResearchEn() {
  return [
    {
      text: 'Research',
      items: [
        { text: 'Related Work', link: '/en/research/related-work' }
      ]
    }
  ]
}

function sidebarWhitepaperEn() {
  return [
    {
      text: 'Whitepaper',
      items: [
        { text: 'Overview', link: '/en/whitepaper/' },
        { text: 'DAG Scheduling', link: '/en/whitepaper/dag-scheduling' },
        { text: 'Memory Management', link: '/en/whitepaper/memory-management' },
        { text: 'Heterogeneous Execution', link: '/en/whitepaper/heterogeneous-execution' },
        { text: 'Performance Analysis', link: '/en/whitepaper/performance-analysis' }
      ]
    }
  ]
}

// Sidebar functions for Chinese
function sidebarGuideZh() {
  return [
    {
      text: '入门指南',
      collapsed: false,
      items: [
        { text: '简介', link: '/zh/guide/' },
        { text: '安装', link: '/zh/guide/installation' },
        { text: '架构', link: '/zh/guide/architecture' }
      ]
    },
    {
      text: '核心主题',
      collapsed: false,
      items: [
        { text: '调度', link: '/zh/guide/scheduling' },
        { text: '内存', link: '/zh/guide/memory' },
        { text: '错误处理', link: '/zh/guide/error-handling' }
      ]
    }
  ]
}

function sidebarAPIZh() {
  return [
    {
      text: '核心 API',
      items: [
        { text: '概述', link: '/zh/api/' }
      ]
    }
  ]
}

function sidebarExamplesZh() {
  return [
    {
      text: '示例',
      items: [
        { text: '概述', link: '/zh/examples/' }
      ]
    }
  ]
}

function sidebarDesignZh() {
  return [
    {
      text: '设计',
      items: [
        { text: '设计哲学', link: '/zh/design/philosophy' }
      ]
    }
  ]
}

function sidebarBenchmarksZh() {
  return [
    {
      text: '性能基准',
      items: [
        { text: '概述', link: '/zh/benchmarks/' }
      ]
    }
  ]
}

function sidebarResearchZh() {
  return [
    {
      text: '研究',
      items: [
        { text: '相关工作', link: '/zh/research/related-work' }
      ]
    }
  ]
}

function sidebarWhitepaperZh() {
  return [
    {
      text: '技术白皮书',
      items: [
        { text: '概述', link: '/zh/whitepaper/' },
        { text: 'DAG 调度', link: '/zh/whitepaper/dag-scheduling' },
        { text: '内存管理', link: '/zh/whitepaper/memory-management' },
        { text: '异构执行', link: '/zh/whitepaper/heterogeneous-execution' },
        { text: '性能分析', link: '/zh/whitepaper/performance-analysis' }
      ]
    }
  ]
}

export default withMermaid(defineConfig({
  base,
  title: 'HTS',
  titleTemplate: ':title | Heterogeneous Task Scheduler',
  description: 'C++17 DAG scheduling library for mixed CPU and GPU workloads',
  cleanUrls: true,
  lastUpdated: true,
  sitemap: {
    hostname: 'https://lessup.github.io/heterogeneous-task-scheduler/'
  },

  head: [
    ['link', { rel: 'icon', type: 'image/svg+xml', href: `${base}logo.svg` }],
    ['link', { rel: 'mask-icon', href: `${base}logo.svg`, color: '#0891b2' }],
    ['meta', { name: 'theme-color', content: '#0891b2' }],
    ['meta', { name: 'og:type', content: 'website' }],
    ['meta', { name: 'og:site_name', content: 'HTS' }],
    ['meta', { name: 'twitter:card', content: 'summary_large_image' }],
    ['meta', { name: 'apple-mobile-web-app-capable', content: 'yes' }],
    ['meta', { name: 'apple-mobile-web-app-status-bar-style', content: 'black' }],
    ['meta', { property: 'og:title', content: 'HTS - Heterogeneous Task Scheduler' }],
    ['meta', { property: 'og:description', content: 'C++17 DAG scheduling library for mixed CPU and GPU workloads' }],
  ],

  markdown: {
    theme: {
      light: 'github-light',
      dark: 'github-dark'
    },
    lineNumbers: true,
    math: true
  },

  mermaid: {
    theme: 'default'
  },

  vite: {
    plugins: [llmstxt()]
  },

  locales: {
    root: {
      label: 'English',
      lang: 'en',
      link: '/en/',
      themeConfig: {
        nav: [
          { text: 'Guide', link: '/en/guide/', activeMatch: '/en/guide/' },
          { text: 'Whitepaper', link: '/en/whitepaper/', activeMatch: '/en/whitepaper/' },
          { text: 'Design', link: '/en/design/', activeMatch: '/en/design/' },
          { text: 'Benchmarks', link: '/en/benchmarks/', activeMatch: '/en/benchmarks/' },
          { text: 'API', link: '/en/api/', activeMatch: '/en/api/' },
          { text: 'GitHub', link: 'https://github.com/LessUp/heterogeneous-task-scheduler' }
        ],
        sidebar: {
          '/en/guide/': sidebarGuideEn(),
          '/en/whitepaper/': sidebarWhitepaperEn(),
          '/en/api/': sidebarAPIEn(),
          '/en/examples/': sidebarExamplesEn(),
          '/en/design/': sidebarDesignEn(),
          '/en/benchmarks/': sidebarBenchmarksEn(),
          '/en/research/': sidebarResearchEn()
        }
      }
    },
    zh: {
      label: '简体中文',
      lang: 'zh-CN',
      link: '/zh/',
      themeConfig: {
        nav: [
          { text: '指南', link: '/zh/guide/', activeMatch: '/zh/guide/' },
          { text: '白皮书', link: '/zh/whitepaper/', activeMatch: '/zh/whitepaper/' },
          { text: '设计', link: '/zh/design/', activeMatch: '/zh/design/' },
          { text: '性能', link: '/zh/benchmarks/', activeMatch: '/zh/benchmarks/' },
          { text: 'API', link: '/zh/api/', activeMatch: '/zh/api/' },
          { text: 'GitHub', link: 'https://github.com/LessUp/heterogeneous-task-scheduler' }
        ],
        sidebar: {
          '/zh/guide/': sidebarGuideZh(),
          '/zh/whitepaper/': sidebarWhitepaperZh(),
          '/zh/api/': sidebarAPIZh(),
          '/zh/examples/': sidebarExamplesZh(),
          '/zh/design/': sidebarDesignZh(),
          '/zh/benchmarks/': sidebarBenchmarksZh(),
          '/zh/research/': sidebarResearchZh()
        },
        outline: { label: '目录' },
        docFooter: { prev: '上一页', next: '下一页' },
        lastUpdated: { text: '最后更新' },
        darkModeSwitchLabel: '外观',
        sidebarMenuLabel: '菜单',
        returnToTopLabel: '返回顶部',
        editLink: {
          pattern: 'https://github.com/LessUp/heterogeneous-task-scheduler/edit/main/website/:path',
          text: '在 GitHub 上编辑此页'
        }
      }
    }
  },

  themeConfig: {
    logo: '/logo.svg',
    siteTitle: 'HTS',

    socialLinks: [
      { icon: 'github', link: 'https://github.com/LessUp/heterogeneous-task-scheduler' }
    ],

    footer: {
      message: 'Released under the MIT License.',
      copyright: 'Copyright © 2024-2026 HTS Contributors'
    },

    search: {
      provider: 'local',
      options: {
        detailedView: true
      }
    },

    editLink: {
      pattern: 'https://github.com/LessUp/heterogeneous-task-scheduler/edit/main/website/:path',
      text: 'Edit this page on GitHub'
    },

    outline: {
      label: 'On this page',
      level: [2, 4]
    },

    docFooter: {
      prev: 'Previous page',
      next: 'Next page'
    },

    darkModeSwitchLabel: 'Appearance',
    sidebarMenuLabel: 'Menu',
    returnToTopLabel: 'Return to top',
    externalLinkIcon: true
  }
}))
