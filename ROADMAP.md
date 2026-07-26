# Roadmap

## v1.0.0-beta.6

- [x] Switch to .po files for translations
- [x] Refactor UI
- [x] Refactor Timetable
- [ ] Refactor the rest of the codebase
  - [x] Use JsonFormat
  - [x] Use std::mutex
  - [x] Add .clang-tidy
  - [ ] Figure something out with changing daysPerWeek or lessonsPerDay deleting user data
  - [x] Remove a dependency on cacert.pem
  - [x] Use the printf syntax of ImGui::Text
  - [x] Add Web.cpp
  - [ ] Doxygen documentation
  - [x] amount -> count in Timetable
- [ ] Unit tests
- [ ] Automatic crash report sending
- [ ] Change the way to make multiple lessons in one cell to groups (a more logical and flexible solution)
- [x] Back up timetables on load
- [ ] Don't make languages separate in releases
- [ ] Use the system language by default
- [ ] Optimise GetRuleCount
- [ ] Switch to nlohmann/json or make JsonFormat standard-compliant and more easily usable

## v1.0.0-beta.7

- [ ] Exporting to HTML
- [ ] Exporting to some image format
- [ ] Switch to RayUI or Qt?
- [ ] Scoring on the GPU
- [ ] Toggle scoring functions in settings
