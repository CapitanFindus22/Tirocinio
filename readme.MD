# Acceleratore hardware emulato

Questa repo contiene il prototipo di acceleratore hardware virtuale che accompagna la mia tesi svolta per completare la laurea triennale in informatica. 

## Contenuto:
- **disp** contiene il file che definisce l'acceleratore stesso secondo il modello QEMU Object Model.
- **library** contiene una libreria che permette all'utente di usare il dispositivo stesso + alcuni test.
- **modulo** contiene il driver associato all'acceleratore.
- **script e altro** contiene script vari tra cui quello per compilare il tutto e per far partire l'emulazione.
- **rootfs, linux_kernel_folder, qemu_folder** sono placeholder rispettivamente per: il filesystem da passare a QEMU (poi combinato con BusyBox), il codice sorgente del kernel Linux (6.14.7), il codice sorgente di QEMU (10.0.2). Nel caso si usino versioni o un architettura diversa vanno aggiustati i vari script.

**N.B.** i file rcS e pci_dev.sh dentro **script e altro** vanno rispettivamente inseriti in **rootfs/etc/init.d** e **rootfs/usr/bin** per creare direttamente il dispositivo associato dentro /dev.