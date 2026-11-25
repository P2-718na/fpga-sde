set windows-shell := ["powershell.exe", "-NoLogo", "-Command"]
set script-interpreter := ["powershell.exe", "-NoLogo", "-ExecutionPolicy", "Bypass", "-File"]

board_host   := env("PYNQ_HOST", "192.168.2.99")
board_user   := env("PYNQ_USER", "xilinx")
board_target := board_user + "@" + board_host
board_dir    := "/home/xilinx/fhn-fpga"
ssh_key      := home_directory() / ".ssh/id_ed25519-xilinx"
ring_result  := "build/results/ring_result.npz"

default:
    @just --list

# Run HLS C-Simulation for net-accel
[group("vitis")]
[working-directory("net-accel")]
csim:
    vitis-run --mode hls --csim --config hls_config.cfg --work_dir build

# Run HLS C-Synthesis for net-accel
[group("vitis")]
[working-directory("net-accel")]
csynth:
    v++ -c --mode hls --config hls_config.cfg --work_dir build

# Run HLS C/RTL Co-simulation for net-accel
[group("vitis")]
[working-directory("net-accel")]
cosim:
    vitis-run --mode hls --cosim --config hls_config.cfg --work_dir build

# Package HLS IP for net-accel
[group("vitis")]
[working-directory("net-accel")]
package:
    vitis-run --mode hls --package --config hls_config.cfg --work_dir build

# Plot HLS C-sim trajectory with gnuplot
[group("vitis")]
plot:
    gnuplot -p scripts/plot-trajectory.gp

alias sim := csim
alias synth := csynth

# Open Vivado GUI (creates vivado/build/fhn_pynq.xpr from Tcl if missing)
[group("vivado")]
[working-directory("vivado/build")]
[script]
[extension("ps1")]
vivado:
    if (-not (Test-Path "fhn_pynq.xpr")) {
        vivado -mode batch -nojournal -nolog -source ../init_project.tcl
        if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    }
    vivado fhn_pynq.xpr

# Build bitstream + HWH into vivado/export/net_accel.{bit,hwh} (strategy: timing | fast | default)
[group("vivado")]
[working-directory("vivado/build")]
bitstream strategy="fast":
    vivado -mode batch -nojournal -nolog -source ../build_bitstream.tcl -tclargs {{strategy}}

# One-time PYNQ-Z2 SSH key + passwordless /usr/local/bin/pynq-python setup
[group("board")]
[script]
[extension("ps1")]
setup-board:
    if (-not (Test-Path "{{ssh_key}}")) {
        New-Item -ItemType Directory -Force -Path (Split-Path -Parent "{{ssh_key}}") | Out-Null
        cmd /c ssh-keygen -t ed25519 -N "" -f "{{ssh_key}}"
        if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    }
    $sshConfig = Join-Path (Split-Path -Parent "{{ssh_key}}") "config"
    if (-not (Test-Path $sshConfig) -or -not (Select-String -Path $sshConfig -SimpleMatch "id_ed25519-xilinx" -Quiet)) {
        Add-Content -Path $sshConfig -Value "`nHost pynq {{board_host}}`n    HostName {{board_host}}`n    User {{board_user}}`n    IdentityFile ~/.ssh/id_ed25519-xilinx`n    IdentitiesOnly yes`n"
    }
    Get-Content "{{ssh_key}}.pub" | ssh {{board_target}} "mkdir -p -m 700 ~/.ssh && cat >> ~/.ssh/authorized_keys && sort -u -o ~/.ssh/authorized_keys ~/.ssh/authorized_keys && chmod 600 ~/.ssh/authorized_keys"
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    scp -i "{{ssh_key}}" scripts/setup_board.sh "{{board_target}}:/tmp/setup_board.sh"
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    ssh -i "{{ssh_key}}" -t {{board_target}} 'sed -i "s/\r$//" /tmp/setup_board.sh && sudo bash /tmp/setup_board.sh && rm -f /tmp/setup_board.sh'
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

# Deploy Python driver modules and vivado/export/net_accel.{bit,hwh} to the board
[group("board")]
[script]
[extension("ps1")]
deploy:
    if (-not (Test-Path "vivado/export/net_accel.bit") -or -not (Test-Path "vivado/export/net_accel.hwh")) {
        throw "Missing vivado/export/net_accel.bit or .hwh. Run 'just bitstream' first."
    }
    ssh -i "{{ssh_key}}" {{board_target}} "mkdir -p {{board_dir}}"
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    $pyFiles = (Get-ChildItem -File python/*.py).FullName
    scp -i "{{ssh_key}}" $pyFiles vivado/export/net_accel.bit vivado/export/net_accel.hwh "{{board_target}}:{{board_dir}}/"
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

# Run bidirectional ring integration on PYNQ and fetch the .npz result
# Stride = how often to read current state
[group("board")]
[script]
[extension("ps1")]
[arg("neurons", short="N", long="neurons")]
[arg("steps", short="s", long="steps")]
[arg("stride", long="stride")]
[arg("dt", long="dt")]
[arg("J", short="J", long="J")]
[arg("a", short="a", long="a")]
[arg("epsilon", short="e", long="epsilon")]
[arg("sigma", long="sigma")]
[arg("seed", long="seed")]
[arg("impulse_width", long="impulse-width")]
[arg("impulse_u", long="impulse-u")]
run-ring neurons="1024" steps="5000" stride="0" dt="0.001" J="0.5" a="1.3" epsilon="0.1" sigma="0.0" seed="12345678" impulse_width="5" impulse_u="1.5":
    $remoteArgs = @(
        "--bitfile net_accel.bit"
        "--neurons={{neurons}}"
        "--steps={{steps}}"
        "--stride={{stride}}"
        "--dt={{dt}}"
        "--J={{J}}"
        "--a={{a}}"
        "--epsilon={{epsilon}}"
        "--sigma={{sigma}}"
        "--seed={{seed}}"
        "--impulse-width={{impulse_width}}"
        "--impulse-u={{impulse_u}}"
        "--out=ring_result.npz"
    ) -join " "
    ssh -i "{{ssh_key}}" {{board_target}} "cd {{board_dir}} && sudo /usr/local/bin/pynq-python run_ring.py $remoteArgs"
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    New-Item -ItemType Directory -Force -Path (Split-Path -Parent "{{ring_result}}") | Out-Null
    scp -i "{{ssh_key}}" "{{board_target}}:{{board_dir}}/ring_result.npz" "{{ring_result}}"
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

# Plot the saved ring simulation .npz result locally
[group("python")]
plot-ring:
    python scripts/plot-ring.py "{{ring_result}}"

# Run host-side Python unit tests
[group("python")]
[working-directory("python")]
test:
    python -m unittest discover -s tests -v


