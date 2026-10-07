prepend_path("PATH","/apps/slurm/default/bin")
prepend_path("MODULEPATH", "/apps/spack/modules/linux-rocky9-x86_64/Core")

load("intel-oneapi-compilers/2024.2.1")

prepend_path("MODULEPATH", "/contrib/spack-stack/spack-stack-1.9.2/envs/ue-oneapi-2024.2.1/install/modulefiles/Core")

load("stack-oneapi/2024.2.1")
load("intel-oneapi-compilers/2023.2.0")
load("jasper/2.0.32")
load("python/3.11")
load("awscli-v2/2.15.53")
