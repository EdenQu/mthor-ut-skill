/**
 * Cursor UT Skill - Professional Unit Test Generation
 * 
 * This skill provides automated unit test generation for:
 * - iOS (Swift/XCTest)
 * - Android (Kotlin/JUnit)
 * - C++ (GoogleTest/GMock)
 * 
 * Commands:
 * - ut_changed [--base=<branch>] - Generate UT for git changed files
 * - ut_path --path=<dir> - Generate UT for files in specified path
 * - ut_files --files="file1,file2" - Generate UT for specific files
 * 
 * @author eden_qu
 * @version 1.0.0
 */

const fs = require('fs');
const path = require('path');

const SKILL_FILES = {
  skill: 'skill.md',
  workflow: 'workflow.md',
  trigger: 'trigger.mdc',
  readme: 'README.md'
};

const PROMPTS = {
  ut_changed: 'prompts/ut_changed.md',
  ut_path: 'prompts/ut_path.md',
  ut_files: 'prompts/ut_files.md'
};

const TEMPLATES = {
  ios: 'templates/ios-test-template.swift',
  android: 'templates/android-test-template.kt',
  cpp: 'templates/cpp-test-template.cpp'
};

function getSkillPath(file) {
  return path.join(__dirname, file);
}

function readSkillFile(file) {
  return fs.readFileSync(getSkillPath(file), 'utf8');
}

module.exports = {
  name: 'mthor-ut-skill',
  version: '1.0.0',
  SKILL_FILES,
  PROMPTS,
  TEMPLATES,
  getSkillPath,
  readSkillFile
};
