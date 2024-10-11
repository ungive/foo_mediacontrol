.PHONY: package

component_name=foo_mediacontrol
dll_filename=$(component_name).dll
dll_x86=.\foobar2000\foo_mediacontrol\Release\$(dll_filename)
dll_x64=.\foobar2000\foo_mediacontrol\x64\Release\$(dll_filename)
out_ext=.fb2k-component

all:

package: $(dll_x86) $(dll_x64)
	rmdir /S /Q package
	md package
	md package\x86
	md package\x64
	copy $(dll_x86) .\package\x86\$(dll_filename)
	copy $(dll_x64) .\package\x64\$(dll_filename)
	powershell Compress-Archive .\package\x86\$(dll_filename) .\package\$(component_name)-x86.zip
	powershell Compress-Archive .\package\x64\$(dll_filename) .\package\$(component_name)-x64.zip
	powershell $$pwd=(Resolve-Path .); Rename-Item -Path "$$pwd\package\$(component_name)-x86.zip" -NewName "$$pwd\package\$(component_name)-x86$(out_ext)"
	powershell $$pwd=(Resolve-Path .); Rename-Item -Path "$$pwd\package\$(component_name)-x64.zip" -NewName "$$pwd\package\$(component_name)-x64$(out_ext)"
