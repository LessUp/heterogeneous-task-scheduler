import { defineConfig } from 'vitepress'

export default defineConfig({
  title: 'HTS',
  titleTemplate: ':title | Heterogeneous Task Scheduler',
  description: 'C++17 DAG scheduling library for mixed CPU and GPU workloads',
  base: '/heterogeneous-task-scheduler/',
  cleanUrls: true,
  lastUpdated: true,
  sitemap: {
    hostname: 'https://lessup.github.io/heterogeneous-task-scheduler/'
  },
  
  head: [
    ['link', { rel: 'icon', type: 'image/svg+xml', href: '/logo.svg' }],
    ['link', { rel: 'mask-icon', href: '/logo.svg', color: '#3EAF7C' }],
    ['meta', { name: 'theme-color', content: '#3EAF7C' }],
    ['meta', { name: 'og:type', content: 'website' }],
    ['meta', { name: 'og:site_name', content: 'HTS' }],
    ['meta', { name: 'twitter:card', content: 'summary_large_image' }],
    ['meta', { name: 'apple-mobile-web-app-capable', content: 'yes' }],
    ['meta', { name: 'apple-mobile-web-app-status-bar-style', content: 'black' }]
  ],

  markdown: {
    theme: {
      light: 'github-light',
      dark: 'github-dark'
    },
    lineNumbers: true,
    math: true
  },

  locales: {
    root: {
      label: 'English',
      lang: 'en',
      themeConfig: {
        nav: [
          { text: 'Overview', link: '/' },
          { text: 'Guide', link: '/guide/', activeMatch: '/guide/' },
          { text: 'Architecture', link: '/guide/architecture' },
          { text: 'API', link: '/api/', activeMatch: '/api/' },
          { text: 'Examples', link: '/examples/', activeMatch: '/examples/' },
          { text: 'GitHub', link: 'https://github.com/LessUp/heterogeneous-task-scheduler' }
        ],
        sidebar: {
          '/guide/': sidebarGuideEn(),
          '/api/': sidebarAPI(),
          '/examples/': sidebarExamples()
        }
      }
    },
    'zh-CN': {
      label: '简体中文',
      lang: 'zh-CN',
      link: '/zh-CN/',
      themeConfig: {
        nav: [
          { text: '概述', link: '/zh-CN/' },
          { text: '指南', link: '/zh-CN/guide/', activeMatch: '/zh-CN/guide/' },
          { text: '架构', link: '/zh-CN/guide/architecture' },
          { text: 'API', link: '/zh-CN/api/', activeMatch: '/zh-CN/api/' },
          { text: '示例', link: '/zh-CN/examples/', activeMatch: '/zh-CN/examples/' },
          { text: 'GitHub', link: 'https://github.com/LessUp/heterogeneous-task-scheduler' }
        ],
        sidebar: {
          '/zh-CN/guide/': sidebarGuideZh(),
          '/zh-CN/api/': sidebarAPIZh(),
          '/zh-CN/examples/': sidebarExamplesZh()
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
})

function sidebarGuideEn() {
  return [
    {
      text: 'Getting Started',
      collapsed: false,
      items: [
        { text: 'Introduction', link: '/guide/' },
        { text: 'Installation', link: '/guide/installation' },
        { text: 'Quick Start', link: '/guide/quickstart' },
        { text: 'Architecture', link: '/guide/architecture' }
      ]
    },
    {
      text: 'Core Topics',
      collapsed: false,
      items: [
        { text: 'Task Graph', link: '/guide/task-graph' },
        { text: 'Scheduling', link: '/guide/scheduling' },
        { text: 'Memory', link: '/guide/memory' },
        { text: 'Error Handling', link: '/guide/error-handling' }
      ]
    }
  ]
}

function sidebarAPI() {
  return [
    {
      text: 'Core API',
      items: [
        { text: 'Overview', link: '/api/' },
        { text: 'Scheduler', link: '/api/scheduler' },
        { text: 'TaskGraph', link: '/api/task-graph' },
        { text: 'TaskBuilder', link: '/api/task-builder' }
      ]
    }
  ]
}

function sidebarExamples() {
  return [
    {
      text: 'Examples',
      items: [
        { text: 'Overview', link: '/examples/' },
        { text: 'Simple DAG', link: '/examples/simple-dag' },
        { text: 'Pipeline', link: '/examples/pipeline' }
      ]
    }
  ]
}

function sidebarGuideZh() {
  return [
    {
      text: '入门指南',
      collapsed: false,
      items: [
        { text: '简介', link: '/zh-CN/guide/' },
        { text: '安装', link: '/zh-CN/guide/installation' },
        { text: '快速开始', link: '/zh-CN/guide/quickstart' },
        { text: '架构', link: '/zh-CN/guide/architecture' }
      ]
    },
    {
      text: '核心主题',
      collapsed: false,
      items: [
        { text: '任务图', link: '/zh-CN/guide/task-graph' },
        { text: '调度', link: '/zh-CN/guide/scheduling' },
        { text: '内存', link: '/zh-CN/guide/memory' },
        { text: '错误处理', link: '/zh-CN/guide/error-handling' }
      ]
    }
  ]
}

function sidebarAPIZh() {
  return [
    {
      text: '核心 API',
      items: [
        { text: '概述', link: '/zh-CN/api/' }
      ]
    }
  ]
}

function sidebarExamplesZh() {
  return [
    {
      text: '示例',
      items: [
        { text: '概述', link: '/zh-CN/examples/' }
      ]
    }
  ]
}
