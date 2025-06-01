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
	mainpkg := denv.NewPackage("github.com\\jurgen-kluft", "cbenchmark")
	mainpkg.AddPackage(cunittestpkg)
	mainpkg.AddPackage(ccorepkg)

	// 'cbenchmark' library
	mainlib := denv.SetupCppLibProject(mainpkg, "cbenchmark")
	mainlib.AddDependencies(ccorepkg.GetMainLib()...)

	// 'cbenchmark' unittest project
	maintest := denv.SetupCppTestProject(mainpkg, "cbenchmark_test")
	maintest.AddDependencies(cunittestpkg.GetMainLib()...)
	maintest.AddDependencies(ccorepkg.GetMainLib()...)
	maintest.AddDependency(mainlib)

	mainpkg.AddMainLib(mainlib)
	mainpkg.AddUnittest(maintest)
	return mainpkg
}
