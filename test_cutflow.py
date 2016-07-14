# import mva.regions
# import mva.categories.hadhad.common
import operator
from mva.samples import Data
from mva.samples import Higgs
from rootpy.tree import Cut

year     = 2015
year_cut = 2016
# mode     = 'Data'
mode     = 'MC'
# mode_mc  = 'gg'
mode_mc  = 'VBF'
# Weighting only affects MC:
weighted = False

if   mode == 'Data':
  cutflow_input = Data(year)
elif mode == 'MC':
  cutflow_input = Higgs(year, mode = mode_mc)

# -------------------------------------------------------------------------------------------------

cuts = []

# MC ntuple splitting
if mode == 'MC':
  if   year_cut == 2015:
    cuts.append(Cut('random_run_number <  284485'))
  elif year_cut == 2016:
    cuts.append(Cut('random_run_number >= 284485'))
# GRL
if mode == 'Data':
  cuts.append(Cut('grl_pass_run_lb == 1'))
# Trigger
if   year_cut == 2015:
  cuts.append(Cut('HLT_tau35_medium1_tracktwo_tau25_medium1_tracktwo_L1TAU20IM_2TAU12IM == 1'))
elif year_cut == 2016:
  cuts.append(Cut('HLT_tau35_medium1_tracktwo_tau25_medium1_tracktwo == 1'))
# Jet trigger
cuts.append(Cut('(jet_0_pt > 70.0) && (abs(jet_0_eta) < 3.2)')) # okay when fabs changed to abs
# Charge
cuts.append(Cut('(abs(ditau_tau0_q) == 1) && (abs(ditau_tau1_q) == 1)'))
# ntracks
cuts.append(Cut('((ditau_tau0_n_tracks == 1) || (ditau_tau0_n_tracks == 3)) && ((ditau_tau1_n_tracks == 1) || (ditau_tau1_n_tracks == 3))')) # okay when more brackets added
# Tau ID
cuts.append(Cut('(n_taus_medium == 2) & (n_taus_tight >= 1) && (ditau_tau0_jet_bdt_medium == 1) && (ditau_tau1_jet_bdt_medium == 1)')) # okay when "==1" added for last two conditions
# Tau pT
cuts.append(Cut('(ditau_tau0_pt > 40) && (ditau_tau1_pt > 30)'))
# OS
cuts.append(Cut('selection_opposite_sign == 1'))
# MET
cuts.append(Cut('selection_met == 1'))
# MET centrality
cuts.append(Cut('selection_met_centrality == 1'))
# deta
cuts.append(Cut('selection_delta_eta == 1'))
# dr
cuts.append(Cut('selection_delta_r == 1'))
# Lepton veto (no BDT)
cuts.append(Cut('selection_lepton_veto == 1'))

# -------------------------------------------------------------------------------------------------

cutflow_output = []
for i in range(len(cuts)):
  cutflow_output.append(cutflow_input.events(weighted = weighted, cuts = reduce(operator.and_, cuts[0:i+1]))[1].value)
for i in cutflow_output:
  print i

# -------------------------------------------------------------------------------------------------

# cuts0_out  = cutflow_input.events(weighted = weighted, cuts = (cuts[0]))[1].value
# cuts1_out  = cutflow_input.events(weighted = weighted, cuts = (cuts[0] & cuts[1]))[1].value
# cuts2_out  = cutflow_input.events(weighted = weighted, cuts = (cuts[0] & cuts[1] & cuts[2]))[1].value
# cuts3_out  = cutflow_input.events(weighted = weighted, cuts = (cuts[0] & cuts[1] & cuts[2] & cuts[3]))[1].value
# cuts4_out  = cutflow_input.events(weighted = weighted, cuts = (cuts[0] & cuts[1] & cuts[2] & cuts[3] & cuts[4]))[1].value
# cuts5_out  = cutflow_input.events(weighted = weighted, cuts = (cuts[0] & cuts[1] & cuts[2] & cuts[3] & cuts[4] & cuts[5]))[1].value
# cuts6_out  = cutflow_input.events(weighted = weighted, cuts = (cuts[0] & cuts[1] & cuts[2] & cuts[3] & cuts[4] & cuts[5] & cuts[6]))[1].value
# cuts7_out  = cutflow_input.events(weighted = weighted, cuts = (cuts[0] & cuts[1] & cuts[2] & cuts[3] & cuts[4] & cuts[5] & cuts[6] & cuts[7]))[1].value
# cuts8_out  = cutflow_input.events(weighted = weighted, cuts = (cuts[0] & cuts[1] & cuts[2] & cuts[3] & cuts[4] & cuts[5] & cuts[6] & cuts[7] & cuts[8]))[1].value
# cuts9_out  = cutflow_input.events(weighted = weighted, cuts = (cuts[0] & cuts[1] & cuts[2] & cuts[3] & cuts[4] & cuts[5] & cuts[6] & cuts[7] & cuts[8] & cuts[9]))[1].value
# cuts10_out = cutflow_input.events(weighted = weighted, cuts = (cuts[0] & cuts[1] & cuts[2] & cuts[3] & cuts[4] & cuts[5] & cuts[6] & cuts[7] & cuts[8] & cuts[9] & cuts[10]))[1].value
# cuts11_out = cutflow_input.events(weighted = weighted, cuts = (cuts[0] & cuts[1] & cuts[2] & cuts[3] & cuts[4] & cuts[5] & cuts[6] & cuts[7] & cuts[8] & cuts[9] & cuts[10] & cuts[11]))[1].value
# cuts12_out = cutflow_input.events(weighted = weighted, cuts = (cuts[0] & cuts[1] & cuts[2] & cuts[3] & cuts[4] & cuts[5] & cuts[6] & cuts[7] & cuts[8] & cuts[9] & cuts[10] & cuts[11] & cuts[12]))[1].value
# 
# print cuts0_out
# print cuts1_out
# print cuts2_out
# print cuts3_out
# print cuts4_out
# print cuts5_out
# print cuts6_out
# print cuts7_out
# print cuts8_out
# print cuts9_out
# print cuts10_out
# print cuts11_out
# print cuts12_out

# -------------------------------------------------------------------------------------------------

# cut0 = Cut('')
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
# cut1 = mva.regions.Q
# cut2 = mva.regions.P1P3
# cut3 = mva.regions.ID_MEDIUM
# cut4 = mva.regions.TRACK_ISOLATION
# cut5 = mva.categories.hadhad.common.LEAD_TAU_40
# cut6 = mva.categories.hadhad.common.SUBLEAD_TAU_30
# cut7 = mva.regions.OS
# cut8 = mva.categories.hadhad.common.MET
# # cut9 = mva.categories.hadhad.common.MET_CENTRALITY
# cut9 = mva.categories.hadhad.common.DETA_TAUS
# cut10 = mva.categories.hadhad.common.DR_TAUS

# cuts.append(Cut(''))
# cuts.append(mva.regions.Q)
# cuts.append(mva.regions.P1P3)
# cuts.append(mva.regions.ID_MEDIUM)
# cuts.append(mva.regions.TRACK_ISOLATION)
# cuts.append(mva.categories.hadhad.common.LEAD_TAU_40)
# cuts.append(mva.categories.hadhad.common.SUBLEAD_TAU_30)
# cuts.append(mva.regions.OS)
# cuts.append(mva.categories.hadhad.common.MET)
# # cuts.append(mva.categories.hadhad.common.MET_CENTRALITY)
# cuts.append(mva.categories.hadhad.common.DETA_TAUS)
# cuts.append(mva.categories.hadhad.common.DR_TAUS)

# -------------------------------------------------------------------------------------------------

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

# cut0_out = higgs.events(weighted=weighted, cuts = (cut0))[1].value
# cut1_out = higgs.events(weighted=weighted, cuts = (cut0 & cut1))[1].value
# cut2_out = higgs.events(weighted=weighted, cuts = (cut0 & cut1 & cut2))[1].value
# cut3_out = higgs.events(weighted=weighted, cuts = (cut0 & cut1 & cut2 & cut3))[1].value
# cut4_out = higgs.events(weighted=weighted, cuts = (cut0 & cut1 & cut2 & cut3 & cut4))[1].value
# cut5_out = higgs.events(weighted=weighted, cuts = (cut0 & cut1 & cut2 & cut3 & cut4 & cut5))[1].value
# cut6_out = higgs.events(weighted=weighted, cuts = (cut0 & cut1 & cut2 & cut3 & cut4 & cut5 & cut6))[1].value
# cut7_out = higgs.events(weighted=weighted, cuts = (cut0 & cut1 & cut2 & cut3 & cut4 & cut5 & cut6 & cut7))[1].value
# cut8_out = higgs.events(weighted=weighted, cuts = (cut0 & cut1 & cut2 & cut3 & cut4 & cut5 & cut6 & cut7 & cut8))[1].value
# cut9_out = higgs.events(weighted=weighted, cuts = (cut0 & cut1 & cut2 & cut3 & cut4 & cut5 & cut6 & cut7 & cut8 & cut9))[1].value
# cut10_out = higgs.events(weighted=weighted, cuts = (cut0 & cut1 & cut2 & cut3 & cut4 & cut5 & cut6 & cut7 & cut8 & cut9 & cut10))[1].value
# cut11_out = higgs.events(weighted=weighted, cuts = (cut0 & cut1 & cut2 & cut3 & cut4 & cut5 & cut6 & cut7 & cut8 & cut9 & cut10 & cut11))[1].value

# -------------------------------------------------------------------------------------------------

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

# print cut0_out
# print cut1_out
# print cut2_out
# print cut3_out
# print cut4_out
# print cut5_out
# print cut6_out
# print cut7_out
# print cut8_out
# print cut9_out
# print cut10_out