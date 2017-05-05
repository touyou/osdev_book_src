# -*- mode: ruby -*-
# vi: set ft=ruby :

Vagrant.configure(2) do |config|
  config.vm.box = "ubuntu14.04"
  config.vm.box_check_update = false
  config.vm.box_url = "https://cloud-images.ubuntu.com/vagrant/trusty/current/trusty-server-cloudimg-amd64-vagrant-disk1.box"
  config.vm.provision :shell, :path => "bootstrap.sh", :privileged   => false
  config.vm.network "forwarded_port", guest: 5900, host: 25900 # for VNC

  config.vm.provider :virtualbox do |vb|
    if ENV['VAGRANT_MEMORY']
      vb.memory = ENV['VAGRANT_MEMORY']
    else
      vb.memory = 2048
    end
    vb.cpus = 2
    vb.customize [
      "modifyvm", :id,
      "--nictype1", "virtio",
      "--natdnshostresolver1", "on",
      "--hwvirtex", "on",
      "--nestedpaging", "on",
      "--largepages", "on",
      "--ioapic", "on",
      "--pae", "on",
      "--paravirtprovider", "kvm",
    ]
  end
end
