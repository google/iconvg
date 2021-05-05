// Copyright 2021 The IconVG Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

//go:build ignore

package main

// gen-release-c.go amalgamates src/c/* into a single release/c/foo.c file.

import (
	"bytes"
	"fmt"
	"io"
	"os"
	"path/filepath"
)

func main() {
	if err := main1(); err != nil {
		os.Stderr.WriteString(err.Error() + "\n")
		os.Exit(1)
	}
}

func main1() error {
	if !cdIconVGRootDirectory() {
		return fmt.Errorf("main: could not find or chdir to the IconVG root directory")
	}

	buf := &bytes.Buffer{}
	remaining, err := os.ReadFile(filepath.FromSlash("src/c/aaa_package.h"))
	if err != nil {
		return err
	}
	for len(remaining) > 0 {
		line := remaining
		remaining = nil
		if i := bytes.IndexByte(line, '\n'); i >= 0 {
			line, remaining = line[:i+1], line[i+1:]
		}

		if !bytes.HasPrefix(line, hashInclude) {
			buf.Write(line)
		} else if err := expand(buf, line); err != nil {
			return err
		} else {
			buf.WriteByte('\n')
		}
	}

	version := "unsupported-snapshot"
	outFilename := filepath.FromSlash("release/c/iconvg-" + version + ".c")
	return os.WriteFile(outFilename, buf.Bytes(), 0o644)
}

var (
	copyright   = []byte(`// Copyright `)
	dquoteNL    = []byte("\"\n")
	hashInclude = []byte(`#include "./`)
	nlNL        = []byte("\n\n")
)

func cdIconVGRootDirectory() bool {
	prevWD, err := os.Getwd()
	if err != nil {
		return false
	}
	for {
		if _, err := os.Stat("iconvg-root-directory.txt"); err == nil {
			break
		}
		if err := os.Chdir(".."); err != nil {
			return false
		}
		if wd, err := os.Getwd(); (err != nil) || (prevWD == wd) {
			return false
		} else {
			prevWD = wd
		}
	}
	return true
}

func expand(w io.Writer, hashIncludeLine []byte) error {
	if !bytes.HasSuffix(hashIncludeLine, dquoteNL) {
		return fmt.Errorf("main: unexpected line %q", hashIncludeLine)
	}
	filename := string(hashIncludeLine[len(hashInclude) : len(hashIncludeLine)-len(dquoteNL)])
	fmt.Fprintf(w, "// -------------------------------- %s", hashIncludeLine)

	src, err := os.ReadFile(filepath.FromSlash("src/c/" + filename))
	if err != nil {
		return err
	}
	if !bytes.HasPrefix(src, copyright) {
		return fmt.Errorf("main: %q did not start with the expected copyright header", filename)
	} else if i := bytes.Index(src, nlNL); i < 0 {
		return fmt.Errorf("main: %q did not start with the expected copyright header", filename)
	} else {
		src = src[i+1:]
	}
	_, err = w.Write(src)
	return err
}
