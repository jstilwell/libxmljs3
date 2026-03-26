import * as path from 'node:path';
import type { NativeBindings } from './types';

// node-gyp-build finds the correct prebuild or falls back to build/Release
const gyp: (dir: string) => NativeBindings = require('node-gyp-build');
const binding: NativeBindings = gyp(path.join(__dirname, '..'));

export default binding;
