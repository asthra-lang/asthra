import type {SidebarsConfig} from '@docusaurus/plugin-content-docs';

const sidebars: SidebarsConfig = {
  stdlibSidebar: [
    'README',
    'import-convention',
    'error-handling',
    'best-practices',
    'collections_design',
    {
      type: 'category',
      label: 'Modules',
      link: {
        type: 'generated-index',
        title: 'Standard Library Modules',
        description: 'Explore the Asthra Standard Library modules.',
        slug: '/stdlib/modules',
      },
      items: [
        'modules/core',
        'modules/json',
        'modules/string',
      ],
    },
  ],
};

export default sidebars; 