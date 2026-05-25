import { defineConfig } from 'vitepress'
import { withMermaid } from 'vitepress-plugin-mermaid'

const rawBase = process.env.VITEPRESS_BASE
const base = rawBase
  ? rawBase.startsWith('/')
    ? rawBase.endsWith('/') ? rawBase : `${rawBase}/`
    : `/${rawBase}/`
  : '/heterogeneous-task-scheduler/'

function guideSidebarEn() {
  return [
    {
      text: 'Getting Started',
      collapsed: false,
      items: [
        { text: 'Introduction', link: '/en/guide/' },
        { text: 'Installation', link: '/en/guide/installation' },
        { text: 'Quick Start', link: '/en/guide/quickstart' },
        { text: 'Architecture', link: '/en/guide/architecture' },
        { text: 'Task Graph', link: '/en/guide/task-graph' },
        { text: 'Scheduling', link: '/en/guide/scheduling' },
        { text: 'Memory', link: '/en/guide/memory' },
        { text: 'Error Handling', link: '/en/guide/error-handling' }
      ]
    }
  ]
}

function apiSidebarEn() {
  return [
    {
      text: 'API',
      items: [
        { text: 'Overview', link: '/en/api/' },
        { text: 'Scheduler', link: '/en/api/scheduler' },
        { text: 'TaskGraph', link: '/en/api/task-graph' },
        { text: 'TaskBuilder', link: '/en/api/task-builder' }
      ]
    }
  ]
}

function examplesSidebarEn() {
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

function guideSidebarZh() {
  return [
    {
      text: '入门指南',
      collapsed: false,
      items: [
        { text: '简介', link: '/zh/guide/' },
        { text: '安装', link: '/zh/guide/installation' },
        { text: '架构', link: '/zh/guide/architecture' },
        { text: '调度', link: '/zh/guide/scheduling' },
        { text: '内存', link: '/zh/guide/memory' },
        { text: '错误处理', link: '/zh/guide/error-handling' }
      ]
    }
  ]
}

function apiSidebarZh() {
  return [
    {
      text: 'API',
      items: [
        { text: '概述', link: '/zh/api/' }
      ]
    }
  ]
}

function examplesSidebarZh() {
  return [
    {
      text: '示例',
      items: [
        { text: '概述', link: '/zh/examples/' }
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
    hostname: 'https://aicl-lab.github.io/heterogeneous-task-scheduler/'
  },
  head: [
    ['link', { rel: 'icon', type: 'image/svg+xml', href: `${base}logo.svg` }],
    ['link', { rel: 'mask-icon', href: `${base}logo.svg`, color: '#0891b2' }],
    ['meta', { name: 'theme-color', content: '#0891b2' }],
    ['meta', { name: 'og:type', content: 'website' }],
    ['meta', { name: 'og:site_name', content: 'HTS' }],
    ['meta', { name: 'twitter:card', content: 'summary_large_image' }],
    ['meta', { property: 'og:title', content: 'HTS - Heterogeneous Task Scheduler' }],
    ['meta', { property: 'og:description', content: 'C++17 DAG scheduling library for mixed CPU and GPU workloads' }],
    ['meta', { property: 'og:image', content: 'https://aicl-lab.github.io/heterogeneous-task-scheduler/og-image.png' }],
    ['meta', { name: 'twitter:image', content: 'https://aicl-lab.github.io/heterogeneous-task-scheduler/og-image.png' }]
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
  locales: {
    root: {
      label: 'English',
      lang: 'en',
      link: '/en/',
      themeConfig: {
        nav: [
          { text: 'Guide', link: '/en/guide/', activeMatch: '/en/guide/' },
          { text: 'API', link: '/en/api/', activeMatch: '/en/api/' },
          { text: 'Examples', link: '/en/examples/', activeMatch: '/en/examples/' },
          { text: 'GitHub', link: 'https://github.com/AICL-Lab/heterogeneous-task-scheduler' }
        ],
        sidebar: {
          '/en/guide/': guideSidebarEn(),
          '/en/api/': apiSidebarEn(),
          '/en/examples/': examplesSidebarEn()
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
          { text: 'API', link: '/zh/api/', activeMatch: '/zh/api/' },
          { text: '示例', link: '/zh/examples/', activeMatch: '/zh/examples/' },
          { text: 'GitHub', link: 'https://github.com/AICL-Lab/heterogeneous-task-scheduler' }
        ],
        sidebar: {
          '/zh/guide/': guideSidebarZh(),
          '/zh/api/': apiSidebarZh(),
          '/zh/examples/': examplesSidebarZh()
        },
        outline: { label: '目录' },
        docFooter: { prev: '上一页', next: '下一页' },
        lastUpdated: { text: '最后更新' },
        darkModeSwitchLabel: '外观',
        sidebarMenuLabel: '菜单',
        returnToTopLabel: '返回顶部',
        editLink: {
          pattern: 'https://github.com/AICL-Lab/heterogeneous-task-scheduler/edit/master/website/:path',
          text: '在 GitHub 上编辑此页'
        }
      }
    }
  },
  themeConfig: {
    logo: '/logo.svg',
    siteTitle: 'HTS',
    socialLinks: [
      { icon: 'github', link: 'https://github.com/AICL-Lab/heterogeneous-task-scheduler' }
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
      pattern: 'https://github.com/AICL-Lab/heterogeneous-task-scheduler/edit/master/website/:path',
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
