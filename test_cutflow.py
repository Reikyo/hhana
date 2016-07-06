from mva.samples import Higgs
import mva.regions
import mva.categories.hadhad.common

#higgs = Higgs(2015, mode='gg')
higgs = Higgs(2015, mode='VBF')

from rootpy.tree import Cut

cut0 = Cut('')
# cut0 = Cut('(abs(ditau_deta) < 1.37) || ((1.52 < abs(ditau_deta)) && (abs(ditau_deta) < 2.5))')
# cut1 = mva.regions.Q
# cut2 = mva.regions.P1P3
# cut3 = mva.regions.ID_MEDIUM
# cut4 = mva.categories.hadhad.common.LEAD_TAU_40
# cut5 = mva.categories.hadhad.common.SUBLEAD_TAU_30
# cut6 = mva.regions.OS
# cut7 = mva.categories.hadhad.common.MET
# cut8 = mva.categories.hadhad.common.DETA_TAUS
# cut9 = mva.categories.hadhad.common.DR_TAUS

# 2016 / 06 / 25 DTemple: Redefined the cuts as follows for the v10 ntuple acceptance challenge:
cut1 = mva.regions.Q
cut2 = mva.regions.P1P3
cut3 = mva.regions.ID_MEDIUM
cut4 = mva.regions.TRACK_ISOLATION
cut5 = mva.categories.hadhad.common.LEAD_TAU_40
cut6 = mva.categories.hadhad.common.SUBLEAD_TAU_30
cut7 = mva.regions.OS
cut8 = mva.categories.hadhad.common.MET
# cut9 = mva.categories.hadhad.common.MET_CENTRALITY
cut9 = mva.categories.hadhad.common.DETA_TAUS
cut10 = mva.categories.hadhad.common.DR_TAUS

weighted = True

# cut0_out = higgs.events(weighted=weighted)[1].value
# cut1_out = higgs.events(weighted=weighted, cuts = (cut1))[1].value
# cut2_out = higgs.events(weighted=weighted, cuts = (cut1 & cut2))[1].value
# cut3_out = higgs.events(weighted=weighted, cuts = (cut1 & cut2 & cut3))[1].value
# cut4_out = higgs.events(weighted=weighted, cuts = (cut1 & cut2 & cut3 & cut4))[1].value
# cut5_out = higgs.events(weighted=weighted, cuts = (cut1 & cut2 & cut3 & cut4 & cut5))[1].value
# cut6_out = higgs.events(weighted=weighted, cuts = (cut1 & cut2 & cut3 & cut4 & cut5 & cut6))[1].value
# cut7_out = higgs.events(weighted=weighted, cuts = (cut1 & cut2 & cut3 & cut4 & cut5 & cut6 & cut7))[1].value
# cut8_out = higgs.events(weighted=weighted, cuts = (cut1 & cut2 & cut3 & cut4 & cut5 & cut6 & cut7 & cut8))[1].value
# cut9_out = higgs.events(weighted=weighted, cuts = (cut1 & cut2 & cut3 & cut4 & cut5 & cut6 & cut7 & cut8 & cut9))[1].value

cut0_out = higgs.events(weighted=weighted, cuts = (cut0))[1].value
cut1_out = higgs.events(weighted=weighted, cuts = (cut0 & cut1))[1].value
cut2_out = higgs.events(weighted=weighted, cuts = (cut0 & cut1 & cut2))[1].value
cut3_out = higgs.events(weighted=weighted, cuts = (cut0 & cut1 & cut2 & cut3))[1].value
cut4_out = higgs.events(weighted=weighted, cuts = (cut0 & cut1 & cut2 & cut3 & cut4))[1].value
cut5_out = higgs.events(weighted=weighted, cuts = (cut0 & cut1 & cut2 & cut3 & cut4 & cut5))[1].value
cut6_out = higgs.events(weighted=weighted, cuts = (cut0 & cut1 & cut2 & cut3 & cut4 & cut5 & cut6))[1].value
cut7_out = higgs.events(weighted=weighted, cuts = (cut0 & cut1 & cut2 & cut3 & cut4 & cut5 & cut6 & cut7))[1].value
cut8_out = higgs.events(weighted=weighted, cuts = (cut0 & cut1 & cut2 & cut3 & cut4 & cut5 & cut6 & cut7 & cut8))[1].value
cut9_out = higgs.events(weighted=weighted, cuts = (cut0 & cut1 & cut2 & cut3 & cut4 & cut5 & cut6 & cut7 & cut8 & cut9))[1].value
cut10_out = higgs.events(weighted=weighted, cuts = (cut0 & cut1 & cut2 & cut3 & cut4 & cut5 & cut6 & cut7 & cut8 & cut9 & cut10))[1].value
# cut11_out = higgs.events(weighted=weighted, cuts = (cut0 & cut1 & cut2 & cut3 & cut4 & cut5 & cut6 & cut7 & cut8 & cut9 & cut10 & cut11))[1].value

# print 'No selection: ', a
# print '      Charge: ', b
# print '     nTracks: ', c
# print '       tauID: ', d
# print '     Leading: ', e
# print '  Subleading: ', f
# print '          OS: ', g
# print '         MET: ', h
# print '        Deta: ', i
# print '          Dr: ', j

print cut0_out
print cut1_out
print cut2_out
print cut3_out
print cut4_out
print cut5_out
print cut6_out
print cut7_out
print cut8_out
print cut9_out
print cut10_out