# NEO-MAXIS Economy Research

## Target executable

- Game: SimCity 2000 Special Edition
- Executable: SIMCITY.EXE
- Architecture: PE32 / Intel i386
- Image base: 0x00400000
- Image size: 0x00163000
- SDK: sc2kfix r11b

## Memory candidates

| Name | Absolute address | RVA | Declared type |
|---|---:|---:|---|
| dwCityFunds | 0x4CA444 | 0x000CA444 | int |
| dwCityBonds | 0x4CC4E8 | 0x000CC4E8 | int |
| dwInterestRateSum | 0x4CA400 | 0x000CA400 | DWORD |
| pBudgetArr | 0x4CC4CC | 0x000CC4CC | budget_t* |
| wArrBondData | 0x4C9EF0 | 0x000C9EF0 | WORD array |
| pIndividualIndTaxRate | 0x4CB3E0 | 0x000CB3E0 | __int16* |

## Budget structure

budget_t:
- iFundMonth[12]
- iCurrentCosts
- iFundingPercent
- iYearToDateCost
- iEstimatedCost

## Relevant functions

- SimulationPrepareBudgetDialog: 0x4015E6
- UpdateBudgetInformation: 0x402D2E

## Verification status

- [x] Executable fingerprint recorded
- [x] SDK memory declarations located
- [ ] Treasury value independently verified
- [ ] Bond value independently verified
- [ ] Interest calculation understood
- [ ] Budget structure layout verified
- [ ] Budget pointer validated
- [ ] Revenue and expenditure fields identified
- [ ] Read-only economy reader implemented

## Safety

No game memory writes are permitted during this research phase.
All memory reads must be validated and performed through safe read operations.
