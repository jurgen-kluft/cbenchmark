package cbenchmark

import (
	denv "github.com/jurgen-kluft/ccode/denv"
	ccore "github.com/jurgen-kluft/ccore/package"
	cunittest "github.com/jurgen-kluft/cunittest/package"
)

// GetPackage returns the package object of 'cbenchmark'
func GetPackage() *denv.Package {
	// Dependencies
	cunittestpkg := cunittest.GetPackage()
	ccorepkg := ccore.GetPackage()

	// The main (cbenchmark) package
	mainpkg := denv.NewPackage("cbenchmark")
	mainpkg.AddPackage(cunittestpkg)
	mainpkg.AddPackage(ccorepkg)

	// 'cbenchmark' library
	mainlib := denv.SetupCppLibProject("cbenchmark", "github.com\\jurgen-kluft\\cbenchmark")
	mainlib.AddDependencies(ccorepkg.GetMainLib()...)

	// 'cbenchmark' unittest project
	maintest := denv.SetupDefaultCppTestProject("cbenchmark_test", "github.com\\jurgen-kluft\\cbenchmark")
	maintest.AddDependencies(cunittestpkg.GetMainLib()...)
	maintest.AddDependencies(ccorepkg.GetMainLib()...)
	maintest.Dependencies = append(maintest.Dependencies, mainlib)

	mainpkg.AddMainLib(mainlib)
	mainpkg.AddUnittest(maintest)
	return mainpkg
}
