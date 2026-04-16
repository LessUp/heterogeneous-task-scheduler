import { defineConfig } from 'vitepress'
import { withPwa } from '@vite-pwa/vitepress'

const config = defineConfig({
  title: 'HTS',
  titleTemplate: ':title | Heterogeneous Task Scheduler',
  description: 'High-performance C++ framework for CPU/GPU task scheduling',
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
    ['meta', { name: 'apple-mobile-web-app-status-bar-style', content: 'black' }],
    ['script', {}, `
      if ('serviceWorker' in navigator) {
        navigator.serviceWorker.register('/sw.js');
      }
    `]
  ],

  markdown: {
    theme: {
      light: 'github-light',
      dark: 'github-dark'
    },
    lineNumbers: true,
    math: true
  },

  themeConfig: {
    logo: '/logo.svg',
    siteTitle: 'HTS',
    
    nav: [
      { text: 'Guide', link: '/guide/', activeMatch: '/guide/' },
      { text: 'API', link: '/api/', activeMatch: '/api/' },
      { text: 'Examples', link: '/examples/', activeMatch: '/examples/' },
      {
        text: 'v1.2.0',
        items: [
          { text: 'Changelog', link: '/changelog' },
          { text: 'Contributing', link: '/contributing' },
          { text: 'GitHub', link: 'https://github.com/LessUp/heterogeneous-task-scheduler' }
        ]
      }
    ],

    sidebar: {
      '/guide/': sidebarGuide(),
      '/api/': sidebarAPI(),
      '/examples/': sidebarExamples()
    },

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

function sidebarGuide() {
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

export default config
