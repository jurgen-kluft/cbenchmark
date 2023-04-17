package cbenchmark

import (
	"github.com/jurgen-kluft/ccode/denv"
)

// GetPackage returns the package object of 'cbase'
func GetPackage() *denv.Package {

	// 'cbenchmark' library
	mainlib := denv.SetupDefaultCppLibProject("cbenchmark", "github.com\\jurgen-kluft\\cbenchmark")

	// 'cbenchmark' test project
	maintest := denv.SetupDefaultCppBenchMarkProject("cbenchmark_test", "github.com\\jurgen-kluft\\cbenchmark")
	maintest.Dependencies = append(maintest.Dependencies, mainlib)

	mainpkg := denv.NewPackage("cbenchmark")
	mainpkg.AddMainLib(mainlib)
	mainpkg.AddUnittest(maintest)
	return mainpkg
}
