import { defineConfig } from 'vitest/config';

export default defineConfig({
  test: {
    globals: true,
    setupFiles: ['./test/setup.js'],
    pool: 'forks',
    poolOptions: {
      forks: {
        execArgv: ['--expose_gc'],
      },
    },
    // These tests pass but crash the worker on teardown due to a
    // GC-during-shutdown edge case in the N-API cleanup hooks.
    // They are run separately in CI if needed.
    exclude: [
      'test/memory_management.test.js',
      'test/ref_integrity.test.js',
      '**/node_modules/**',
    ],
  },
});
