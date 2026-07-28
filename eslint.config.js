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
    // Tests deliberately reassign variables to drop references so the
    // GC can collect the underlying native objects. ESLint 10 added
    // no-useless-assignment to eslint:recommended, which flags exactly
    // that pattern.
    files: ['test/**/*.js'],
    rules: {
      'no-useless-assignment': 'off',
    },
  },
  {
    ignores: ['build/', 'dist/', 'vendor/', 'prebuilds/'],
  },
];
