#!/usr/bin/env bash
set -euo pipefail

# Wrap PYNQ's isolated virtualenv + XRT environment so `sudo` SSH commands can import pynq.
cat >/usr/local/bin/pynq-python <<'EOF'
#!/usr/bin/env bash
set -e
source /etc/profile.d/pynq_venv.sh
if [ -f /etc/profile.d/xrt_setup.sh ]; then
    source /etc/profile.d/xrt_setup.sh
fi
exec python3 "$@"
EOF
chmod 755 /usr/local/bin/pynq-python

echo "xilinx ALL=(root) NOPASSWD: /usr/local/bin/pynq-python" >/etc/sudoers.d/pynq-python
chmod 440 /etc/sudoers.d/pynq-python
visudo -cf /etc/sudoers.d/pynq-python

mkdir -p /home/xilinx/fhn-fpga
chown -R xilinx:xilinx /home/xilinx/fhn-fpga

sudo -u xilinx sudo -n /usr/local/bin/pynq-python -c "import pynq; print('PYNQ ready:', pynq.__file__)"
