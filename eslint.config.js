const js = require('@eslint/js');
const prettier = require('eslint-config-prettier');
const globals = require('globals');

module.exports = [
  js.configs.recommended,
  prettier,
  {
    languageOptions: {
      ecmaVersion: 2022,
      sourceType: 'commonjs',
      globals: {
        ...globals.node,
        ...globals.jest, // vitest globals are compatible with jest globals
      },
    },
    rules: {
      'no-unused-vars': ['error', { argsIgnorePattern: '^_', varsIgnorePattern: '^_' }],
    },
  },
  {
    files: ['vitest.config.js'],
    languageOptions: {
      sourceType: 'module',
    },
  },
  {
    ignores: ['build/', 'dist/', 'vendor/', 'prebuilds/'],
  },
];
