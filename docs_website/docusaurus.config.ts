import {themes as prismThemes} from 'prism-react-renderer';
import type {Config} from '@docusaurus/types';
import type * as Preset from '@docusaurus/preset-classic';

// This runs in Node.js - Don't use client-side code here (browser APIs, JSX...)

const config: Config = {
  title: 'Asthra Language',
  tagline: 'The World\'s First Production-Ready Language for AI Code Generation Excellence',
  favicon: 'img/asthra_icon.svg',

  // Future flags, see https://docusaurus.io/docs/api/docusaurus-config#future
  future: {
    v4: true, // Improve compatibility with the upcoming Docusaurus v4
  },

  // Set the production url of your site here
  url: 'https://your-docusaurus-site.example.com',
  // Set the /<baseUrl>/ pathname under which your site is served
  // For GitHub pages deployment, it is often '/<projectName>/'
  baseUrl: '/',

  // GitHub pages deployment config.
  // If you aren't using GitHub pages, you don't need these.
  organizationName: 'asthra-lang', // Usually your GitHub org/user name.
  projectName: 'asthra', // Usually your repo name.

  onBrokenLinks: 'throw',
  onBrokenMarkdownLinks: 'warn',

  // Even if you don't use internationalization, you can use this field to set
  // useful metadata like html lang. For example, if your site is Chinese, you
  // may want to replace "en" with "zh-Hans".
  i18n: {
    defaultLocale: 'en',
    locales: ['en'],
  },

  presets: [
    [
      'classic',
      {
        docs: false, // Disable the default docs plugin
        blog: {
          showReadingTime: true,
          feedOptions: {
            type: ['rss', 'atom'],
            xslt: true,
          },
          // Please change this to your repo.
          // Remove this to remove the "edit this page" links.
          editUrl:
            'https://github.com/asthra-lang/asthra/tree/main/docs_website/',
          // Useful options to enforce blogging best practices
          onInlineTags: 'warn',
          onInlineAuthors: 'warn',
          onUntruncatedBlogPosts: 'warn',
        },
        theme: {
          customCss: './src/css/custom.css',
        },
      } satisfies Preset.Options,
    ],
  ],
  plugins: [
    [
      '@docusaurus/plugin-content-docs',
      {
        id: 'contributor',
        path: 'docs/contributor',
        routeBasePath: 'contributor',
        sidebarPath: require.resolve('./sidebars_contributor.ts'),
        editUrl: 'https://github.com/asthra-lang/asthra/tree/main/docs_website/',
      },
    ],
    [
      '@docusaurus/plugin-content-docs',
      {
        id: 'spec',
        path: 'docs/spec',
        routeBasePath: 'spec',
        sidebarPath: require.resolve('./sidebars_spec.ts'),
        editUrl: 'https://github.com/asthra-lang/asthra/tree/main/docs_website/',
      },
    ],
    [
      '@docusaurus/plugin-content-docs',
      {
        id: 'user-manual',
        path: 'docs/user-manual',
        routeBasePath: 'user-manual',
        sidebarPath: require.resolve('./sidebars_user_manual.ts'),
        editUrl: 'https://github.com/asthra-lang/asthra/tree/main/docs_website/',
      },
    ],
    [
      '@docusaurus/plugin-content-docs',
      {
        id: 'stdlib',
        path: 'docs/stdlib',
        routeBasePath: 'stdlib',
        sidebarPath: require.resolve('./sidebars_stdlib.ts'),
        editUrl: 'https://github.com/asthra-lang/asthra/tree/main/docs_website/',
      },
    ],
  ],

  themeConfig: {
    // Replace with your project's social card
    image: 'img/asthra_social_card.svg',
    navbar: {
      title: 'Asthra Language',
      logo: {
        alt: 'Asthra Language Logo',
        src: 'img/asthra_logo.svg',
      },
      items: [
        {
          to: '/contributor',
          label: 'Contributor',
          position: 'left',
          activeBaseRegex: `/contributor/`,
        },
        {
          to: '/spec',
          label: 'Spec',
          position: 'left',
          activeBaseRegex: `/spec/`,
        },
        {
          to: '/user-manual',
          label: 'User Manual',
          position: 'left',
          activeBaseRegex: `/user-manual/`,
        },
        {
          to: '/stdlib',
          label: 'Standard Library',
          position: 'left',
          activeBaseRegex: `/stdlib/`,
        },
        {to: '/blog', label: 'Blog', position: 'left'},
        {
          href: 'https://github.com/asthra-lang/asthra',
          label: 'GitHub',
          position: 'right',
        },
      ],
    },
    footer: {
      style: 'dark',
      links: [
        {
          title: 'Docs',
          items: [
            {label: 'Contributor', to: '/contributor'},
            {label: 'Spec', to: '/spec'},
            {label: 'User Manual', to: '/user-manual'},
            {label: 'Standard Library', to: '/stdlib'},
          ],
        },
        {
          title: 'Community',
          items: [
            {
              label: 'Stack Overflow',
              href: 'https://stackoverflow.com/questions/tagged/asthra',
            },
            {
              label: 'Issues',
              href: 'https://github.com/asthra-lang/asthra/issues',
            },
            {
              label: 'Discussions',
              href: 'https://github.com/asthra-lang/asthra/discussions',
            },
          ],
        },
        {
          title: 'More',
          items: [
            {
              label: 'Blog',
              to: '/blog',
            },
            {
              label: 'GitHub',
              href: 'https://github.com/asthra-lang/asthra',
            },
          ],
        },
      ],
      copyright: `Copyright © ${new Date().getFullYear()} Asthra Language. Built with Docusaurus.`,
    },
    prism: {
      theme: prismThemes.github,
      darkTheme: prismThemes.dracula,
    },
  } satisfies Preset.ThemeConfig,
};

export default config;
