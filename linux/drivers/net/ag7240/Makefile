#
# Makefile for Ar7240 gigabit MAC and Phys
#
obj-$(CONFIG_AG7240)			+=	ag7240_mod.o

obj-phy-$(CONFIG_AR7240_S26_PHY) 	+= ar7240_s26_phy.o
obj-phy-$(CONFIG_AR7242_RGMII_PHY)      += athrf1_phy.o
obj-phy-$(CONFIG_AR7242_S16_PHY)        += athrs16_phy.o
obj-phy-$(CONFIG_AR7242_VIR_PHY)        += athr_vir_phy.o
obj-phy-$(CONFIG_ATHRS_QOS)             += athrs_qos.o

ag7240_mod-objs			:=	$(obj-phy-y) ag7240.o python_vlan_igmp.o

